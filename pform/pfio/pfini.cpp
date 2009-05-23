// pfini.cpp
//
// implements the INI type processing for PFGW

#include "pfiopch.h"
#include <stdio.h>
#include <string.h>

int _pfini_stricmp(const char *s1,const char *s2)
{
#if !defined (_MSC_VER)
	static PFString ss1,ss2;
	ss1=s1;
	ss2=s2;
	return ss1.CompareNoCase(ss2);
#else
	return _stricmp(s1, s2);
#endif
}

#if defined (_MSC_VER)
// static data, shared by the threads
CRITICAL_SECTION PFIni::m_sCS;
// Note "bugs" in Win95/NT3.51 force us to start from -1 for InterlockedIncrement and InterlockedDecrement
// to work correctly.  The "bug" is that 0 is returned correctly, but positive/negative values are not
// returned as the correct value, but they are returned as a positive or a negative.  
LONG PFIni::m_CntCS = -1;
#endif

PFPtrArray<PFIniEntry>	*PFIni::m_pDataArray;
int PFIni::m_nRefCount;
char *PFIni::m_szFileName;
time_t PFIni::tt_lastFlushTime;
PFString *PFIni::m_pCurrentFileLineExpressionString;

PFIniEntry::PFIniEntry(const PFString *pSection, const PFString *pKey,const PFString *pData)
  : m_pSection(0), m_pKey(0), m_pData(0), m_pLine(0)
{
	m_pSection = new PFString;
	m_pKey = new PFString;
	m_pData = new PFString;
	m_pLine = new PFString;

	if(pSection)
		*m_pSection = *pSection;
	if(pKey)
		*m_pKey = *pKey;
	if(pData)
		*m_pData = *pData;
}

PFIniEntry::~PFIniEntry()
{
	delete m_pData;
	m_pData = 0;
	delete m_pKey;
	m_pKey = 0;
	delete m_pSection;
	m_pSection = 0;
	delete m_pLine;
	m_pLine = 0;
}

// returns "current" Key, and possibly sets the Key (if pNewKey is not null)
PFString Key_s;
const PFString &PFIniEntry::Key(const PFString *pNewKey)
{
	if (!pNewKey)
		return *m_pKey;
	Key_s = *m_pKey;
	*m_pKey = *pNewKey;
	return Key_s;
}

// returns "current" Data, and possibly sets the Data (if pNewData is not null)
PFString Data_s;
const PFString &PFIniEntry::Data (const PFString *pNewData)
{
	if (!pNewData)
		return *m_pData;
	Data_s = *m_pData;
	*m_pData = *pNewData;
	return Data_s;
}

// returns "current" Data, and possibly sets the Data (if pNewSection is not null)
PFString Section_s;
const PFString &PFIniEntry::Section (const PFString *pNewSection)
{
	if (!pNewSection)
		return *m_pSection;
	Section_s = *m_pSection;
	*m_pSection = *pNewSection;
	return Section_s;
}

// returns "current" Data, and possibly sets the Data (if pNewLine is not null)
PFString Line_s;
const PFString &PFIniEntry::Line (const PFString *pNewLine)
{
	if (!pNewLine)
		return *m_pLine;
	Line_s = *m_pLine;
	*m_pLine = *pNewLine;
	return Line_s;
}

PFIni::PFIni(const char* FileName) 
  : m_szCurrentSection(NULL), m_szCurrentFileReadSection(NULL),
    m_nCurFileLineNum(0), m_nLastFileLineNum(0),
    m_dwSectionStart(0), m_dwSectionEnd(0), m_bDirty(false), m_bValid(false), m_bProcessing(false)
{
	CS_Init();
	sfalse = "false";
	strue = "true";
	m_nRefCount++;

	m_szCurrentFileReadSection = new char[1];
	*m_szCurrentFileReadSection = 0;

	if (m_nRefCount == 1)
	{
		tt_lastFlushTime = time(0);
		if (!FileName)
			FileName = "pfgw.ini";
		m_szFileName = new char[strlen(FileName)+1];
		strcpy(m_szFileName, FileName);

		FILE *fp = fopen(FileName, "rt");
		if (!fp)
			fp = _CreateIniFile();

		// If we still can't open, it could be due to lack of permissions, or something
		// as stupid as having a directory call pfgw.ini.  Whatever the reason, we
		// need to WARN the user, but not crash.  The program CAN continue forward, 
		// however, any restart ability will NOT be there (i.e. ALL of the PFIni
		// functions will "gracefully" fail.
		if (!fp)
		{
			// Make sure this appears on both the console version, and the GUI version (since prints are "cached" in the GUI
			PFOutput::EnableOneLineForceScreenOutput();
			PFPrintfStderr("WARNING! PFGW was not able to open for writing or create the %s file.\n", FileName);
			PFOutput::EnableOneLineForceScreenOutput();
			PFPrintfStderr("PFGW can (and will) continue processing, but there will be NO resume capabilities!\n");
			PFOutput::EnableOneLineForceScreenOutput();
			PFPrintfStderr("\n");
			return;
		}

		m_pDataArray = new PFPtrArray<PFIniEntry>(PFBoolean::b_true);

		for (int i = 0; !feof(fp); i++)
		{
			PFIniEntry *p = new PFIniEntry;
			int Ret = _GetNextLine(fp, p);
			if (Ret != EOF)
				m_pDataArray->Add(p);
			else
				delete p;
		}
		fclose(fp);
		_SetCurrentSection("PFGW");
	}
	m_bValid = true;
}

PFIni::~PFIni()
{
	// make sure the disk is up to date.
	ForceFlush();
	CS_Free();
	delete[] m_szCurrentFileReadSection;
	m_szCurrentFileReadSection = 0;
	delete[] m_szCurrentSection;
	m_szCurrentSection = 0;
	m_nRefCount--;

	if (!m_nRefCount)
	{
		delete[] m_szFileName;
		m_szFileName = 0;
		delete m_pDataArray;
		m_pDataArray = 0;
	}
}

FILE *PFIni::_CreateIniFile()
{
	// Private function.  Do not syncronize.
	FILE *fp = fopen(m_szFileName, "wt");
	if (!fp)
	{
		//throw "Error, can not open ini file";
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr("WARNING, can not create the ini file!!!\n");
		return 0;
	}
	fprintf(fp, "[PFGW]\n");
	fprintf(fp, "MODE=PRP\n");

	// flush file
	fclose(fp);
	fp = fopen(m_szFileName, "rt");
	m_bDirty = false;
	return fp;
}

int PFIni::_GetNextLine(FILE *fp, PFIniEntry *p)
{
	// Private function.  Do not syncronize.
	char Line[65000], *cp, *cpData;
	fgets(Line, sizeof(Line), fp);
	if (feof(fp))
		return EOF;

	cp = strtok(Line, "\r\n");
	// fixes a bug where we GP if there are "blank" lines in the .ini file.
	if (!cp)
		return _GetNextLine(fp, p);
	PFString s(cp);
    p->Line(&s);
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '[')
	{
		cp = strtok(cp, "[]");
		PFString s1(cp);
		p->Section(&s1);
		delete[] m_szCurrentFileReadSection;
		m_szCurrentFileReadSection = new char[strlen(cp)+1];
		strcpy(m_szCurrentFileReadSection, cp);
		return _GetNextLine(fp, p);
	}
	PFString Sec(m_szCurrentFileReadSection);
	p->Section(&Sec);
	if (*cp == '#')
		return 0;
	cpData = strchr(cp, '=');
	if (!cpData)
		return _GetNextLine(fp, p);
	*cpData++ = 0;
	PFString Key(cp);
	p->Key(&Key);
	PFString Data(cpData);
	p->Data(&Data);
	if (!p->Key().CompareNoCase("CurLineNum") && !p->Section().CompareNoCase("PFGW"))
		m_nCurFileLineNum = atoi(cpData);
	return 0;
}

PFIni::PFIniErr PFIni::SetCurrentSection(const char *SectionName)
{
	if (!m_bValid)
		return eIniNotOpened;
	CS_Lock();
	PFIniErr e = _SetCurrentSection(SectionName);
	CS_Release();
	return e;
}

PFIni::PFIniErr PFIni::_SetCurrentSection(const char *SectionName)
{
	// Private function.  Do not syncronize.
	if (m_szCurrentSection && !_pfini_stricmp(m_szCurrentSection, SectionName))
		return eOK;
	delete[] m_szCurrentSection;
	m_szCurrentSection = 0;
	bool bFound = false;

	for (m_dwSectionEnd = m_dwSectionStart = 0; m_dwSectionStart < m_pDataArray->GetSize(); m_dwSectionStart++)
	{
		if (!m_pDataArray->operator[](m_dwSectionStart)->Section().CompareNoCase(SectionName))
		{
			bFound=true;
			break;
		}
	}
	if (!bFound)
	{
		//return eSectionNotFound;	 Nope, now we create a new section.
		m_dwSectionStart = m_dwSectionEnd = m_pDataArray->GetSize()-1;
	}
	for (m_dwSectionEnd = m_dwSectionStart+1; m_dwSectionEnd < m_pDataArray->GetSize(); m_dwSectionEnd++)
	{
		if (m_pDataArray->operator[](m_dwSectionEnd)->Section().CompareNoCase(SectionName))
			break;
	}
	m_szCurrentSection = new char[strlen(SectionName)+1];
	strcpy(m_szCurrentSection, SectionName);
	return eOK;
}

void PFIni::GetCurrentSection(PFString &s)
{
	if (!m_bValid)
	{
		s="";
		return;
	}
	CS_Lock();
	_GetCurrentSection(s);
	CS_Release();
}

void PFIni::_GetCurrentSection(PFString &s)
{
	// Private function.  Do not syncronize.
	if (!m_szCurrentSection)
		s = "";
	else
		s = m_szCurrentSection;
}

PFIni::PFIniErr PFIni::GetIniString(PFString *pOutStr, PFString *pKeyName, PFString *pDefault, bool bAddIfNotExist)
{
	if (!m_bValid)
	{
		if (pDefault)	// since pDefault "defaults" to NULL (default paramter of this function)
			*pOutStr = *pDefault;
		else
			*pOutStr = "";
		return eIniNotOpened;
	}
	CS_Lock();
	PFIniErr e = _GetIniString(pOutStr, pKeyName, pDefault, bAddIfNotExist);
	CS_Release();
	return e;
}

PFIni::PFIniErr PFIni::_GetIniString(PFString *pOutStr, PFString *pKeyName, PFString *pDefault, bool bAddIfNotExist)
{
	// Private function.  Do not syncronize.
	if (pDefault)
		*pOutStr = *pDefault;
	else
		*pOutStr = "";
	if (!m_szCurrentSection)
		return eSectionNotSet;

	for (DWORD i = m_dwSectionStart; i < m_dwSectionEnd; i++)
	{
		if (!m_pDataArray->operator[](i)->Key().CompareNoCase(*pKeyName))
		{
			*pOutStr = m_pDataArray->operator[](i)->Data();
			return eOK;
		}
	}
	if (!bAddIfNotExist)
		return eEntryNotFound;

	PFIniEntry *p = new PFIniEntry;
	PFString s(m_szCurrentSection);
	p->Section(&s);
	p->Key(pKeyName);
	p->Data(pDefault);
	m_pDataArray->InsertAt(p, m_dwSectionEnd++);
	// We have added an entry, so the INI file is dirty.
	m_bDirty = true;
	// Flush (if supposed to flush)
	_FlushIfTimedOut();
	*pOutStr = *pDefault;
	return eOK;
}

PFIni::PFIniErr PFIni::GetIniInt(int *pOutInt, PFString *pKeyName, int nDefault, bool bAddIfNotExist)
{
	if (!m_bValid)
	{
		*pOutInt = nDefault;
		return eIniNotOpened;
	}
	CS_Lock();
	PFIniErr e = _GetIniInt(pOutInt, pKeyName, nDefault, bAddIfNotExist);
	CS_Release();
	return e;
}

PFString _GetIniInt_s;
PFIni::PFIniErr PFIni::_GetIniInt(int *pOutInt, PFString *pKeyName, int nDefault, bool bAddIfNotExist)
{
	*pOutInt = nDefault;
	if (!m_szCurrentSection)
		return eSectionNotSet;
	_GetIniInt_s.Set((long int)nDefault);	// to avoid bleating (should really be an int proto)
	CS_Lock();
	PFIniErr e = _GetIniString(&_GetIniInt_s, pKeyName, &_GetIniInt_s, bAddIfNotExist);
	CS_Release();
	if (e == eEntryNotFound)
		return eEntryNotFound;
	*pOutInt = atoi(LPCTSTR(_GetIniInt_s));
	return eOK;
}

PFIni::PFIniErr PFIni::GetIniBool(bool *pOutBool, PFString *pKeyName, bool bDefault, bool bAddIfNotExist)
{
	if (!m_bValid)
	{
		*pOutBool = bDefault;
		return eIniNotOpened;
	}
	CS_Lock();
	PFIniErr e = _GetIniBool(pOutBool, pKeyName, bDefault, bAddIfNotExist);
	CS_Release();
	return e;
}

PFIni::PFIniErr PFIni::_GetIniBool(bool *pOutBool, PFString *pKeyName, bool bDefault, bool bAddIfNotExist)
{
	*pOutBool = bDefault;
	if (!m_szCurrentSection)
		return eSectionNotSet;
	PFString s, *ps;
	ps = bDefault==true?&strue:&sfalse;
	CS_Lock();
	PFIniErr e = _GetIniString(&s, pKeyName, ps, bAddIfNotExist);
	CS_Release();
	if (e == eEntryNotFound)
		return eEntryNotFound;

	if (!_pfini_stricmp((LPCSTR)s, "false") || !_pfini_stricmp((LPCSTR)s, "0"))
		*pOutBool = false;
	else
		*pOutBool = true;
	return eOK;
}

PFIni::PFIniErr PFIni::SetIniString(PFString *pSetTo, PFString *pKeyName)
{
	if (!m_bValid)
		return eIniNotOpened;
	CS_Lock();
	PFIniErr e = _SetIniString(pSetTo, pKeyName);
	CS_Release();
	return e;
}

PFIni::PFIniErr PFIni::_SetIniString(PFString *pSetTo, PFString *pKeyName)
{
	// Private function.  Do not syncronize.
	if (!m_szCurrentSection)
		return eSectionNotSet;

	for (DWORD i = m_dwSectionStart; i < m_dwSectionEnd; i++)
	{
		if (!m_pDataArray->operator[](i)->Key().CompareNoCase(*pKeyName))
		{
			if (m_pDataArray->operator[](i)->Data().CompareNoCase(*pSetTo))
			{
				m_pDataArray->operator[](i)->Data(pSetTo);
				m_bDirty = true;
				_FlushIfTimedOut();
			}
			return eOK;
		}
	}
	PFIniEntry *p = new PFIniEntry;
	PFString s(m_szCurrentSection);
	p->Section(&s);
	p->Key(pKeyName);
	p->Data(pSetTo);
	m_pDataArray->InsertAt(p, m_dwSectionEnd++);
	m_bDirty = true;
	_FlushIfTimedOut();
	return eOK;
}

PFIni::PFIniErr PFIni::SetIniInt (int nSetTo, PFString *pKeyName)
{
	if (!m_bValid)
		return eIniNotOpened;
	CS_Lock();
	PFIniErr e = _SetIniInt (nSetTo, pKeyName);
	CS_Release();
	return e;
}

PFString _SetIniInt_s;
PFIni::PFIniErr PFIni::_SetIniInt (int nSetTo, PFString *pKeyName)
{
	_SetIniInt_s.Set((long int)nSetTo);
	CS_Lock();
	PFIniErr e = _SetIniString(&_SetIniInt_s, pKeyName);
	CS_Release();
	return e;
}

PFIni::PFIniErr PFIni::SetIniBool (bool bSetTo, PFString *pKeyName)
{
	if (!m_bValid)
		return eIniNotOpened;
	CS_Lock();
	PFIniErr e = _SetIniBool(bSetTo, pKeyName);
	CS_Release();
	return e;
}

PFIni::PFIniErr PFIni::_SetIniBool (bool bSetTo, PFString *pKeyName)
{
	PFString *p = &sfalse;
	if (bSetTo)
		p = &strue;
	CS_Lock();
	PFIniErr e = _SetIniString(p, pKeyName);
	CS_Release();
	return e;
}

PFIni::PFIniErr PFIni::DeleteIniKey(PFString *pKeyName)
{
	if (!m_bValid)
		return eIniNotOpened;
	CS_Lock();
	for (DWORD i = m_dwSectionStart; i < m_dwSectionEnd; i++)
	{
		if (!m_pDataArray->operator[](i)->Key().CompareNoCase(*pKeyName))
		{
			m_pDataArray->RemoveAt(i);
			m_bDirty = true;
			_FlushIfTimedOut();
			CS_Release();
			return eOK;
		}
	}
	CS_Release();
	return eEntryNotFound;
}

PFIni::PFIniErr PFIni::DeleteIniSection(PFString *pSectionName)
{
	if (!m_bValid)
		return eIniNotOpened;
	bool bFound = false;
	DWORD i;
	CS_Lock();
	for (i = 0; i < m_pDataArray->GetSize(); i++)
	{
		if (!m_pDataArray->operator[](i)->Section().CompareNoCase(*pSectionName))
		{
			bFound=true;
			break;
		}
	}
	if (!bFound)
	{
		CS_Release();
		return eSectionNotFound;
	}
	while (!m_pDataArray->operator[](i)->Section().CompareNoCase(*pSectionName))
	{
		m_pDataArray->RemoveAt(i);
		m_bDirty = true;
	}
	if (!_pfini_stricmp(m_szCurrentSection, LPCTSTR(*pSectionName)))
	{
		delete[] m_szCurrentSection;
		m_szCurrentSection=0;
		m_dwSectionEnd = m_dwSectionStart = 0;
	}
	_FlushIfTimedOut();
	CS_Release();
	return eOK;
}

void PFIni::_FlushIfTimedOut()
{
	// Private function.  Do not syncronize.
	time_t t = time(0);
	// This should only happen if the user is screwing with the clock.  We will save  this one time, and then the
	// 30 second check below should start working (until the user screws with the clock again ;)
	if (t < (tt_lastFlushTime-6))  // +6)) the +6 was a bug.  It caused any prp less than 6 seconds to simply not be cached.
		_ForceFlush();
	if ( (t - tt_lastFlushTime) > 30)	// have at least a 30 seconds expired?
		_ForceFlush();
}

void PFIni::ForceFlush()
{
	if (!m_bValid)
		return;
	CS_Lock();
	_ForceFlush();
	CS_Release();
}

void PFIni::_ForceFlush()
{
	tt_lastFlushTime = time(0);
	if (m_nCurFileLineNum != m_nLastFileLineNum)
	{
		PFString s, s1("CurLineNum");
		_GetCurrentSection(s);
		_SetCurrentSection("PFGW");
		_SetIniInt(m_nCurFileLineNum, &s1);
		m_nLastFileLineNum = m_nCurFileLineNum;
		m_bDirty=true;
		_SetCurrentSection(s);
	}
	// Update the expression also.
	if (m_pCurrentFileLineExpressionString)
	{
		_SetExprChecksum(m_pCurrentFileLineExpressionString);
		// don't set dirty bit here.  When line number is changed, the dirty bit gets set.
		// this code should not mess with that.  I currently don't track last checksum vs.
		// this checksum to determine if it changed or not.
	}
	if (!m_bDirty)
		// No need to write, so don't
		return;
	FILE *fp = fopen(m_szFileName, "wt");
	if (!fp)
		// Can't write!!!
		return;
	PFString CurSection;
	for (DWORD i = 0; i < m_pDataArray->GetSize(); i++)
	{
		if (m_pDataArray->operator[](i)->Section().CompareNoCase(CurSection))
		{
			if (i)
				fprintf(fp, "\n");
			fprintf(fp, "[%s]\n", LPCTSTR(m_pDataArray->operator[](i)->Section()));
			CurSection = m_pDataArray->operator[](i)->Section();
		}
		if (m_pDataArray->operator[](i)->Key() != "")
			fprintf(fp, "%s=%s\n", LPCTSTR(m_pDataArray->operator[](i)->Key()), LPCTSTR(m_pDataArray->operator[](i)->Data()));
		else
			fprintf(fp, "%s\n", LPCTSTR(m_pDataArray->operator[](i)->Line()));
	}
	fclose(fp);
	m_bDirty = false;
}

void PFIni::IncFileLineNum()
{
	CS_Lock();
	m_nCurFileLineNum++;
	if (m_bValid)
		_FlushIfTimedOut();
	CS_Release();
}

void PFIni::SetFileLineNum(int LineNum)
{
	CS_Lock();
	m_nCurFileLineNum = LineNum;
	if (m_bValid)
		_FlushIfTimedOut();
	CS_Release();
}

int  PFIni::GetFileLineNum()
{
	return m_nCurFileLineNum;
}

void PFIni::SetFileName(PFString *pFName)
{
	if (!m_bValid)
		return;
	PFString s, s1("CurFileName");
	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	_SetIniString(pFName, &s1);
	_SetCurrentSection(s);
	_FlushIfTimedOut();
	CS_Release();
}

PFString* PFIni::GetFileName()
{
	PFString s, s1("CurFileName"), s2(""), *pRet = new PFString;
	if (!m_bValid)
		return pRet;
	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	_GetIniString(pRet, &s1, &s2, true);
	_SetCurrentSection(s);
	CS_Release();
	return pRet;
}

void PFIni::SetOutputFileName(PFString *pFName)
{
	if (!m_bValid)
		return;

	PFString s, s1("OutFileName");

	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	_SetIniString(pFName, &s1);
	_SetCurrentSection(s);
	_FlushIfTimedOut();
	CS_Release();
}

PFString *PFIni::GetOutputFileName()
{
	PFString s, s1("OutFileName"), s2(""), *pRet = new PFString;
	if (!m_bValid)
		return pRet;

	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	_GetIniString(pRet, &s1, &s2, true);
	_SetCurrentSection(s);
	CS_Release();
	return pRet;
}

void PFIni::SetFileProcessing(bool bProcessing)
{
	m_bProcessing = bProcessing;
	if (!m_bValid)
		return;
	PFString s, s1("CurFileProcessing");

	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	_SetIniBool(bProcessing, &s1);
	_SetCurrentSection(s);
	_FlushIfTimedOut();
	CS_Release();
}

bool PFIni::GetFileProcessing()
{
	if (!m_bValid)
		return m_bProcessing;
	PFString s, s1("CurFileProcessing");

	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	bool b;
	_GetIniBool(&b, &s1, true, true);
	_SetCurrentSection(s);
	CS_Release();
	return b;
}

// I know there is crc stuff in pflib, but I have placed this CCITT crc32 function here
#define CRC_32 0xedb88320L    // CRC-32 polynomial
static uint32 crctab[256];
static bool   bcrcInit;
static uint32 onecrc (int item)
{
    uint32 accum = 0;
    for (int i = 8; i > 0; i--)
    {
        if ((item ^ accum) & 0x0001)
            accum = (accum >> 1) ^ CRC_32;
        else
            accum >>= 1;
        item >>= 1;
    }
    return (accum);
}

static void mkcrctab (void)
{
    int i;
    for (i = 0; i < 256; i++)
        crctab[i] = onecrc (i);
	bcrcInit=true;
}

inline uint32 updcrc32 (uint32 crc_val, unsigned char c) 
{
	return crctab[(int)((crc_val)^(c))&0xff] ^ ((crc_val) >> 8); 
}

uint32 Crc32buf (const char *buf, int size)
{
	if (!bcrcInit)
		mkcrctab();
	uint32 Crc32 = 0xFFFFFFFFL;
    for (int i = 0; i < size; i++)
        Crc32 = updcrc32 (Crc32, *buf++);
	return ~Crc32;
}

void PFIni::SetExprChecksum(PFString *pCurrentExpression)
{
	if (!m_bValid)
		return;
	CS_Lock();
	_SetExprChecksum(pCurrentExpression);
	_FlushIfTimedOut();
	CS_Release();
}

void PFIni::_SetExprChecksum(PFString *pCurrentExpression)
{
	// Ok create the ckecksum
	uint32 Crc=0;
	PFString sCheckSum;
	if (pCurrentExpression && *pCurrentExpression != "")
	{
		sCheckSum = *pCurrentExpression;
		if (sCheckSum.GetLength()>60)
		{
			PFString sBegin=sCheckSum.Left(27);
			PFString sEnd=sCheckSum.Right(27);
			sCheckSum=sBegin+"....."+sEnd;
		}
		char cpTmp[120];
		strcpy(cpTmp, LPCTSTR(sCheckSum));
		Crc = Crc32buf(cpTmp, strlen(cpTmp));
	}
	// now store this into the ini file
	PFString s, s1("CurLineExpr");
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	_SetIniString(&sCheckSum, &s1);
	s1="CurLineChecksum";
	_SetIniInt(Crc, &s1);
	_SetCurrentSection(s);
}

bool PFIni::CompareExprChecksum(PFString *pCurrentExpression, PFString *pStoredExpression)
{
	if (!m_bValid)
		return false;
	PFString sCheckSum, sCheckSumWanted;
	uint32 Crc;
	PFString s, s1("CurLineExpr");
	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	_GetIniString(&sCheckSum, &s1);
	s1="CurLineChecksum";
	int nCrc;
	_GetIniInt(&nCrc, &s1);
	Crc = (uint32)nCrc;
	_SetCurrentSection(s);
	CS_Release();

	if (sCheckSum == "")
	{
		//if (!pCurrentExpression || *pCurrentExpression == "")
		if (!nCrc)	// Allow a "forced" setting of CurLineExpr and CurLineChecksum being blank as being OK.
			return true;
		return false;
	}

	sCheckSumWanted = *pCurrentExpression;
	if (sCheckSumWanted.GetLength()>60)
	{
		PFString sBegin=sCheckSumWanted.Left(27);
		PFString sEnd=sCheckSumWanted.Right(27);
		sCheckSumWanted=sBegin+"....."+sEnd;
	}
	*pCurrentExpression = sCheckSumWanted;
	*pStoredExpression = sCheckSum;
	char cpTmp[120];
	strcpy(cpTmp, LPCTSTR(sCheckSumWanted));

	if (*pCurrentExpression == sCheckSum)
		return true;

//	CrcWanted = Crc32buf(cpTmp, strlen(cpTmp));

//	if (Crc != CrcWanted)
		return false;
//	return true;
}

void PFIni::GetDefaultSettings(PFString *pDefault)
{
	if (!m_bValid)
	{
		*pDefault = "";
		return;
	}
	PFString s, s1("DefaultSettings");
	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	PFString sSetTo;
	_GetIniString(pDefault, &s1, &sSetTo, true);
	_SetCurrentSection(s);
	CS_Release();
}

void PFIni::AssignPointerOfCurrentExpression(PFString *p)
{
	// if (!m_bValid)  // for this function, we really don't care, since it does NOT tough anything to do with the file.
	m_pCurrentFileLineExpressionString = p;
}

bool PFIni::IsExprChecksumNull()
{
	if (!m_bValid)
		return true;
	PFString sCheckSum;
	PFString s, s1("CurLineExpr");
	CS_Lock();
	_GetCurrentSection(s);
	_SetCurrentSection("PFGW");
	_GetIniString(&sCheckSum, &s1);
	s1="CurLineChecksum";
	int nCrc;
	_GetIniInt(&nCrc, &s1);
	_SetCurrentSection(s);
	CS_Release();

	if (sCheckSum == "" && nCrc == 0)
		return true;
	return false;
}
