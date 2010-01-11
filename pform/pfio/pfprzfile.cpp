#include "pfiopch.h"
#include <string.h>
#include "stdtypes.h"
#include "pfprzfile.h"

extern bool g_bVerbose;

extern char s_Line[ABCLINELEN];  // located in pfabcdfile.cpp

extern bool DeCompress_bits_From_PrZ_setup(uint32 BitLevel, uint32 _EscapeLevel, uint64 _OffsetK, uint32 _Base, uint32 _nvalsleft, char *cpp, bool _bSkipEvens, bool _bNewPGenFile);
extern bool DeCompress_bits_From_PrZ(FILE *fpIn, char *cpp, bool bIgnoreOutput);


PFPrZFile::PFPrZFile(const char* FileName)
	: PFABCFile(FileName)
{
	m_i64Accum = 0;
	m_bReadNextLineFromFile = false;
	m_bIgnoreOutput = false;
	fclose(m_fpInputFile);
	m_fpInputFile = fopen(FileName, "rb");
	m_SigString = "PrZ_ABCD File";
}

void PFPrZFile::CutOutFirstLine()
{
	// Ok, cut our our [line2] stuff, rebuild the line, and let ABCFile::ProccessFirstLine have at it.
	char Line2[ABCLINELEN];
	char *cp = strchr(s_Line, '[');
	if (!cp)
		throw "Error, Not a valid PrZ_ABCD Sieve file, Can't find a [ char in the first line";
	strcpy(Line2, cp);
	char *cp1 = strchr(Line2, ']');
	if (!cp1)
		throw "Error, Not a valid PrZ_ABCD Sieve file, Can't find a ] char in the first line";
	*cp1++ = 0;

	// Now put the end of the line back on the original first line (over write the [...] stuff)
	strcpy(cp, cp1);

	// eat the ABCD down to a ABC
	memmove (&s_Line[3], &s_Line[4], strlen(&s_Line[4]));

	// We already have the first line, so don't hit the file again until we pass over this data.
	m_bReadNextLineFromFile = false;

	cp = &Line2[1];
	while (*cp == ' ' || *cp == '\t')
		++cp;

	strcpy(m_ABCLookingLine, cp);

	m_i64Accum = _atoi64(cp);  // I know this is non-portable,  I am simply getting the logic done now, and not concerned about it.

	if (m_i64Accum == 0 && *cp != '0')
		throw("Error, Not a valid PrZ_ABCD Sieve file, argument 1 in [] format not valid");
	cp = strchr(cp, ' ');

	// Ready to roll.
}

void PFPrZFile::LoadFirstLine()
{
	PrZ_File_Header PrZHead;
	fread(&PrZHead, 1, sizeof(PrZHead), m_fpInputFile);

	PrZ_Section_Header_Base *pFileHead;
	if (PrZHead.PrZ_IsFermFactABCD)
	{
	    PFPrintfLog("Recognized ABCZ (Fermfact) Sieve file: \n");
		pFileHead = new PrZ_FermFact_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
	}
	else if (PrZHead.PrZ_IsAPSieveABCD)
	{
	    PFPrintfLog("Recognized ABCZ (APSieve) Sieve file: \n");
		pFileHead = new PrZ_APSieve_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
	}
	else if (PrZHead.PrZ_IsNewPGen)
	{
	    PFPrintfLog("Recognized ABCZ (NewPGen) Sieve file: \n");
		throw "Error, wrong constructor was called!\n";
	}
	else
	{
	    PFPrintfLog("Recognized ABCZ (Generic ABCD) Sieve file: \n");
		pFileHead = new PrZ_Generic_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
	}

	uint64 xx;
	if (!pFileHead->GetValues(m_MinNum, m_MaxNum, prz_nvalsleft, xx))
	{
#if defined (_MSC_VER)
		m_MaxNum = 0x7FFFFFFFFFFFFFFF;
#else
		m_MaxNum = 0x7FFFFFFFFFFFFFFFLL;
#endif
		m_MinNum = pFileHead->KOffset();
	}

	sprintf (s_Line, "%s", pFileHead->PrZ_GetFirstLine());
	CutOutFirstLine();

	// do count this "first" line, we have to "reset" to line 0 numbering.
	m_nCurrentLineNum = 0;
	m_nCurrentPhysicalLineNum = 0;

	// Now use PFABCFile to process the line.
	PFABCFile::ProcessFirstLine(s_Line);

	// don't count this "first" line, we have to "reset" to line 0 numbering.
	m_nCurrentLineNum = 1;

	char cpp[ABCLINELEN];
	DeCompress_bits_From_PrZ_setup(PrZHead.PrZ_Bits+3, PrZHead.Prz_ESCs, pFileHead->KOffset(), pFileHead->getPrZ_Base(), (uint32)prz_nvalsleft, cpp, PrZHead.PrZ_SkipEvens, PrZHead.PrZ_IsNewPGen);
	delete pFileHead;
}

PFPrZFile::~PFPrZFile()
{
	// Nothing to do.
}

int PFPrZFile::ReadLine(char *_Line, int sizeofLine)
{
	if (!m_bReadNextLineFromFile)
	{
		// the first line of data is actually embedding in the first line (not in the second line like most other sieve formats).
		// So we should NOT hit the file until after we have processed this stored information.
		m_bReadNextLineFromFile = true;	// Next time, we read from the file
		strncpy(_Line, m_ABCLookingLine, sizeofLine);
		_Line[sizeofLine-1] = 0;
		return prz_nvalsleft == 0;
	}
	// Load new line, compute delta, make a "fake" line, and return it.

	if (!DeCompress_bits_From_PrZ(m_fpInputFile, _Line, m_bIgnoreOutput))
	{
		fclose(m_fpInputFile);
		return true;
	}
	++m_nCurrentPhysicalLineNum;
	return false;
}

int PFPrZFile::SeekToLine(int LineNumber)
{
	// turn "off" the output of the line into ABC format.  All we need to do is to read the
	// lines, and update all accumulators.  Turning off output speeds up the processing
	// significantly.
	m_bIgnoreOutput = true;

	m_i64Accum = 0;
	m_bReadNextLineFromFile = false;

	fseek(m_fpInputFile, 0, SEEK_SET);
	if (m_pIni)
		m_pIni->SetFileProcessing(true);
	m_bEOF = false;
	PrZ_Section_Header_Base *pFileHead=0;
	try
	{
		PrZ_File_Header PrZHead;
		fread(&PrZHead, 1, sizeof(PrZHead), m_fpInputFile);
		if (PrZHead.PrZ_IsFermFactABCD)
			pFileHead = new PrZ_FermFact_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
		else if (PrZHead.PrZ_IsAPSieveABCD)
			pFileHead = new PrZ_APSieve_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
		else if (PrZHead.PrZ_IsNewPGen)
			throw "Error, wrong constructor was called!\n";
		else
			pFileHead = new PrZ_Generic_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
		uint64 xx;
		if (!pFileHead->GetValues(m_MinNum, m_MaxNum, prz_nvalsleft, xx))
		{
#if defined (_MSC_VER)
			m_MaxNum = 0x7FFFFFFFFFFFFFFF;
#else
			m_MaxNum = 0x7FFFFFFFFFFFFFFFLL;
#endif
			m_MinNum = pFileHead->KOffset();
		}
		sprintf (s_Line, "%s", pFileHead->PrZ_GetFirstLine());
		CutOutFirstLine();
		m_nCurrentLineNum = 0;
		m_nCurrentPhysicalLineNum = 0;
		PFABCFile::ProcessFirstLine(s_Line);

		char cpp[ABCLINELEN];
		DeCompress_bits_From_PrZ_setup(PrZHead.PrZ_Bits+3, PrZHead.Prz_ESCs, pFileHead->KOffset(), pFileHead->getPrZ_Base(), (uint32)prz_nvalsleft, cpp, PrZHead.PrZ_SkipEvens, PrZHead.PrZ_IsNewPGen);

	}
	catch(char *s)
	{
		if (g_bVerbose)
		{
			PFOutput::EnableOneLineForceScreenOutput();
			PFPrintfStderr("Warning, [%s] parsing line will try to continue\n", s);
		}
		m_bEOF = true;
		m_bIgnoreOutput = false;
		delete pFileHead;
		return e_eof;
	}
#if defined (_DEBUG)
	m_bIgnoreOutput = false;
#endif
	while (m_nCurrentLineNum < LineNumber)
	{
		if (ReadLine(Line,sizeof(Line))) {
			if (m_pIni)
				m_pIni->SetFileProcessing(false);
			m_bEOF = true;
			return e_eof;
		}
		m_nCurrentLineNum++;
	}
	if (m_pIni)
		m_pIni->SetFileLineNum(m_nCurrentLineNum);

	m_bIgnoreOutput = false;
	delete pFileHead;
	return e_ok;
}


PFPrZ_newpgen_File::PFPrZ_newpgen_File(const char* FileName)
	: PFNewPGenFile(FileName)
{
	m_i64Accum = 0;
	m_nAccum = 0;
	m_bReadNextLineFromFile = false;
	m_bIgnoreOutput = false;
	fclose(m_fpInputFile);
	m_fpInputFile = fopen(FileName, "rb");
	m_SigString = "PrZ_NewPGen file: ";
}

void PFPrZ_newpgen_File::LoadFirstLine()
{
	PrZ_File_Header PrZHead;
	fread(&PrZHead, 1, sizeof(PrZHead), m_fpInputFile);

	PrZ_Section_Header_Base *pFileHead;
	if (PrZHead.PrZ_IsFermFactABCD)
		throw "Error, wrong constructor was called!\n";
	else if (PrZHead.PrZ_IsAPSieveABCD)
		throw "Error, wrong constructor was called!\n";
	else if (PrZHead.PrZ_IsNewPGen)
		pFileHead = new PrZ_NewPGen_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
	else
		throw "Error, wrong constructor was called!\n";

	uint64 xx;
	if (!pFileHead->GetValues(m_MinNum, m_MaxNum, prz_nvalsleft, xx))
	{
#if defined (_MSC_VER)
		m_MaxNum = 0x7FFFFFFFFFFFFFFF;
#else
		m_MaxNum = 0x7FFFFFFFFFFFFFFFLL;
#endif
		m_MinNum = pFileHead->KOffset();
	}

	sprintf (s_Line, "%s", pFileHead->PrZ_GetFirstLine());

	// do count this "first" line, we have to "reset" to line 0 numbering.
	m_nCurrentLineNum = 0;
	m_nCurrentPhysicalLineNum = 0;

	// Now use PFABCFile to process the line.
	PFNewPGenFile::ProcessFirstLine(s_Line, pFileHead->KOffset(), pFileHead->getPrZ_Base());

	// don't count this "first" line, we have to "reset" to line 0 numbering.
	m_nCurrentLineNum = 1;

	DeCompress_bits_From_PrZ_setup(PrZHead.PrZ_Bits+3, PrZHead.Prz_ESCs, pFileHead->KOffset(), pFileHead->getPrZ_Base(), (uint32)prz_nvalsleft, m_NPGLookingLine, PrZHead.PrZ_SkipEvens, PrZHead.PrZ_IsNewPGen);
	delete pFileHead;
}

PFPrZ_newpgen_File::~PFPrZ_newpgen_File()
{
	// Nothing to do.
}

int PFPrZ_newpgen_File::ReadLine(char *_Line, int sizeofLine)
{
	if (!m_bReadNextLineFromFile)
	{
		// the first line of data is actually embedding in the first line (not in the second line like most other sieve formats).
		// So we should NOT hit the file until after we have processed this stored information.
		m_bReadNextLineFromFile = true;	// Next time, we read from the file
		strncpy(_Line, m_NPGLookingLine, sizeofLine);
		_Line[sizeofLine-1] = 0;
		return prz_nvalsleft == 0;
	}
	// Load new line, compute delta, make a "fake" line, and return it.

	if (!DeCompress_bits_From_PrZ(m_fpInputFile, _Line, m_bIgnoreOutput))
	{
		fclose(m_fpInputFile);
		return true;
	}
	++m_nCurrentPhysicalLineNum;
	return false;
}

int PFPrZ_newpgen_File::SeekToLine(int LineNumber)
{
	// turn "off" the output of the line into ABC format.  All we need to do is to read the
	// lines, and update all accumulators.  Turning off output speeds up the processing
	// significantly.
	m_bIgnoreOutput = true;

	fseek(m_fpInputFile, 0, SEEK_SET);
	if (m_pIni)
		m_pIni->SetFileProcessing(true);
	m_bEOF = false;
	PrZ_Section_Header_Base *pFileHead=0;
	try
	{
		PrZ_File_Header PrZHead;
		fread(&PrZHead, 1, sizeof(PrZHead), m_fpInputFile);
		if (PrZHead.PrZ_IsFermFactABCD)
			pFileHead = new PrZ_FermFact_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
		else if (PrZHead.PrZ_IsAPSieveABCD)
			pFileHead = new PrZ_APSieve_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
		else if (PrZHead.PrZ_IsNewPGen)
			throw "Error, wrong constructor was called!\n";
		else
			pFileHead = new PrZ_Generic_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
		uint64 xx;
		if (!pFileHead->GetValues(m_MinNum, m_MaxNum, prz_nvalsleft, xx))
		{
#if defined (_MSC_VER)
			m_MaxNum = 0x7FFFFFFFFFFFFFFF;
#else
			m_MaxNum = 0x7FFFFFFFFFFFFFFFLL;
#endif
			m_MinNum = pFileHead->KOffset();
		}
		sprintf (s_Line, "%s", pFileHead->PrZ_GetFirstLine());
		m_nCurrentLineNum = 0;
		m_nCurrentPhysicalLineNum = 0;
		PFNewPGenFile::ProcessFirstLine(s_Line, pFileHead->KOffset(), pFileHead->getPrZ_Base());
	}
	catch(char *s)
	{
		if (g_bVerbose)
		{
			PFOutput::EnableOneLineForceScreenOutput();
			PFPrintfStderr("Warning, [%s] parsing line will try to continue\n", s);
		}
		m_bEOF = true;
		m_bIgnoreOutput = false;
		delete pFileHead;
		return e_eof;
	}
	while (m_nCurrentLineNum < LineNumber)
	{
		char Line[1024];
		if (ReadLine(Line,sizeof(Line))) {
			if (m_pIni)
				m_pIni->SetFileProcessing(false);
			m_bEOF = true;
			return e_eof;
		}
		m_nCurrentLineNum++;
	}
	if (m_pIni)
		m_pIni->SetFileLineNum(m_nCurrentLineNum);

	m_bIgnoreOutput = false;
	delete pFileHead;
	return e_ok;
}
