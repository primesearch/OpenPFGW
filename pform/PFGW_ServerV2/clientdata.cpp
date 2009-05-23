#include "stdafx.h"
#include "math.h"
#include <process.h>
#include <direct.h>
#include <io.h>

#include "clientdata.h"
#include "tcp_ver2_xfer.h"
#include "pfgw_zlib.h"
#include "pffile.h"

// External data needed for linkages to PFGW libraries (for first-line file processing)
unsigned __int64 g_u64ResidueVal;
int g_nIterationCnt;
bool volatile g_bExitNow;
bool g_bVerbose;
char g_ModularSieveString[80];
bool g_bTestingMode;
int g_ExtraSQFree;
int g_PFGW_Mod_Checking;
int g_CompositeAthenticationLevel;
PFIni *g_pIni;
bool g_bWinPFGW_Verbose;

double log_2 = log(2.0);

CClientData::CClientData()
{
	m_nClients = 0;
    m_nCurClient = 0;
	m_Clients = NULL;
	m_nMaxNumClients = 0;
    m_iSeq = 0x101;

	m_nMaxNumClients = 100;
    m_Clients = new ClientData[m_nMaxNumClients+1];
	memset(m_Clients, 0, sizeof(ClientData)*(m_nMaxNumClients+1));
	for (uint32 i = 1; i < m_nMaxNumClients; ++i)
	{
		m_Clients[i].nWip = -8888888;
		m_Clients[i].dAveRate = 1;
	}
	InitializeCriticalSection(&m_ClientCritical);
	_mkdir ("data");
	m_bLoggingOut = false;
}

CClientData::~CClientData()
{
	DeleteCriticalSection(&m_ClientCritical);
    delete[] m_Clients;
}


void CClientData::AddNewClient(const char *Name)
{
    if (m_nClients == m_nMaxNumClients)
    {
        uint32 i;
	    m_nMaxNumClients += 100;
	    ClientData *p = new ClientData[m_nMaxNumClients];
	    memset(p, 0, sizeof(ClientData)*(m_nMaxNumClients));
	    for (i = 1; i < m_nMaxNumClients; ++i)
		{
		    p[i].nWip = -8888888;
			p[i].dAveRate = 1;
		}

	    for (i = 1; i < m_nClients; ++i)
		    memcpy(&p[i], &m_Clients[i], sizeof(m_Clients[i]));

	    delete[] m_Clients;
	    m_Clients = p;
    }
    m_nClients++;
	m_nCurClient = m_nClients;

	char Ini[256];
	sprintf (Ini, "data/%s/clientstats.ini", Name);
    strcpy(m_Clients[m_nCurClient].ClientName, Name);
	if (!_access(Ini, 0))
	{
		char Buf[200];
		GetPrivateProfileString("Stats", "Score", "0.0", Buf, sizeof(Buf), Ini);
		m_Clients[m_nCurClient].Score = atof(Buf);
		GetPrivateProfileString("Stats", "AveRate", "0.0", Buf, sizeof(Buf), Ini);
		m_Clients[m_nCurClient].dAveRate = atof(Buf);
		m_Clients[m_nCurClient].tLastContact = GetPrivateProfileInt("Stats", "tLastContact", time(0), Ini);
		m_Clients[m_nCurClient].nWip = GetPrivateProfileInt("Stats", "nWip", 0, Ini);
		m_Clients[m_nCurClient].nDone = GetPrivateProfileInt("Stats", "nDone", 0, Ini);
	}
	else
	{
		m_Clients[m_nCurClient].Score = 0;
		m_Clients[m_nCurClient].dAveRate = 1;
		m_Clients[m_nCurClient].tLastContact = time(0) - 2;
		m_Clients[m_nCurClient].nWip = 0;
		m_Clients[m_nCurClient].nDone = 0;

	}
}

uint32 CClientData::FindMachine(const char*Name)
{
    if (m_nClients == 0)
        return 0xFFFFFFFF;

    if (!_stricmp(Name, m_Clients[m_nCurClient].ClientName))
        return m_nCurClient;
    for (uint32 i = 1; i <= m_nClients; ++i)
    {
        if (!_stricmp(Name, m_Clients[i].ClientName))
        {
            m_nCurClient = i;
            return i;
        }
    }
    return 0xFFFFFFFF;
}

HANDLE CClientData::ClientStats_Begin(const char *Name, char *pDataPacket)
{
	m_pAccum = new AccumData;
	memset(m_pAccum, 0, sizeof(AccumData));
	m_pAccum->pCClientDataObject = this; // needed by the thread to later recontact about score.
	strncpy(m_pAccum->ClientName, Name, 39);
	m_pAccum->ClientName[39] = 0;
    _strupr(m_pAccum->ClientName);
	m_pAccum->CurClient = m_nCurClient;

    char *cp=m_pAccum->ClientName;

	sprintf (m_pAccum->ClientDir, "data/%s", cp);

	// "fixup" the name (i.e. remove any \ or / chars, converting them to _'s
	char *cpp = m_pAccum->ClientDir;
	cpp += 4;
	while (*(++cpp))
	{
		if (*cpp == '\\' || *cpp == '/')
			*cpp = '_';
	}
	_mkdir(m_pAccum->ClientDir);

    uint16 crc = 0;
    while (*cp)
        crc += *cp++;
    DWORD Val = m_iSeq++;
    Val <<= 16;
    Val += crc;

	m_hCurrentHandle = (HANDLE)Val;
	m_bLoggingOut = false;

	char FName[256];
	sprintf (FName, "%s/current.FName", m_pAccum->ClientDir);
	FILE *in = fopen(FName, "rt");
	m_pAccum->ClientFName[0] = 0;
	if (in)
	{
		fgets(m_pAccum->ClientFName, sizeof(m_pAccum->ClientFName), in);
		fclose(in);
		strtok(m_pAccum->ClientFName, "\r\n");
	}

    return m_hCurrentHandle;
}

void CClientData::ComputeScores()
{
	double Accum=0.0, Score = 0.0;
	int nCnt = 0;
	if (m_pAccum->m_nComps)
	{
		for (uint32 i = 0; i < m_pAccum->m_nComps; ++i)
		{
			uint32 n, k;
			if (sscanf(m_pAccum->m_Comps[i], "%d*2^%d+1", &n, &k) == 2)
			{
				Accum += k;
				++nCnt;
			}
		}
	}
	if (m_pAccum->m_nPrimes)
	{
		for (uint32 i = 0; i < m_pAccum->m_nPrimes; ++i)
		{
			uint32 n, k;
			if (sscanf(m_pAccum->m_Primes[i], "%d*2^%d+1", &n, &k) == 2)
			{
				Accum += k;
				++nCnt;
			}
		}
	}

	if (nCnt)
	{
		// Give "Proth" reduction scores
		// Formula is:   1/4^(lg2(1200000/ave)) * nCnt;
		double Ave = Accum / nCnt;
		Score = 1/pow(4,log(1200000.0/Ave)/log_2);
		Score *= nCnt;
	}
	else
	{
		// Give "Generic" reduction scores
		// Not sure how to yet.  We might need an expression parser to to this!!!  However, we might simply
		// use "wall-clock" time
	}
	AddNewScore(m_pAccum->ClientName, m_pAccum->CurClient, Score);
}

void CClientData::AddNewScore(const char *pClientName, uint32 ClientIdx, double Score)
{
	uint32 Idx = ClientIdx;
	if (_stricmp(pClientName, m_Clients[Idx].ClientName))
		uint32 Idx = FindMachine(pClientName);
	if (Idx != 0xFFFFFFFF)
	{
		m_Clients[Idx].Score += Score;

		char Ini[256];
		sprintf (Ini, "data/%s", pClientName);
		_mkdir(Ini);
		sprintf (Ini, "data/%s/clientstats.ini", pClientName);

		char Buf[200];
		sprintf (Buf, "%0.9f", m_Clients[Idx].Score);
		WritePrivateProfileString("Stats", "Score", Buf, Ini);
		sprintf (Buf, "%0.4f", m_Clients[Idx].dAveRate);
		WritePrivateProfileString("Stats", "AveRate", Buf, Ini);
		sprintf (Buf, "%d", m_Clients[Idx].tLastContact);
		WritePrivateProfileString("Stats", "tLastContact", Buf, Ini);
		sprintf (Buf, "%d", m_Clients[Idx].nWip);
		WritePrivateProfileString("Stats", "nWip", Buf, Ini);
		sprintf (Buf, "%d", m_Clients[Idx].nDone);
		WritePrivateProfileString("Stats", "nDone", Buf, Ini);
		// Flush the ini file (needed under Win95)
		WritePrivateProfileString(NULL, NULL, NULL, Ini);
	}
	else
		MessageBox(0, "Error, cant find client to update stats", "error", 0);
}

void CClientData::LoadExistingWork()
{
	char FName[128];
	sprintf (FName, "%s/current.wip", m_pAccum->ClientDir);
	FILE *in = fopen(FName, "rb");
	if (!in)
		return;
	int Len = _filelength(_fileno(in));
	// Probably NOT needed here, due to we account for it in the getmoredata
	sprintf (FName, "%s/missing.wip", m_pAccum->ClientDir);
	FILE *in1 = fopen(FName, "rb");
	if (in1)
		Len += _filelength(_fileno(in1));
	m_cpExistingWorkBuf = new char[Len + 11];
	memset(&m_cpExistingWorkBuf[Len], 0, 10);
	if (in1)
	{
		fread(m_cpExistingWorkBuf, 1, _filelength(_fileno(in1)), in1);
		fread(&m_cpExistingWorkBuf[_filelength(_fileno(in1))], 1, _filelength(_fileno(in)), in);
		fclose(in1);
		// clean up the missing.wip file (in the client's folder).  We 
		// MAY remake it very shortly, but for now, we assume it is cleaned up.
		remove(FName); 
	}
	else
		fread(m_cpExistingWorkBuf, 1, Len, in);
	fclose(in);

	char *cp = m_cpExistingWorkBuf;
	char *cp1 = strchr(m_cpExistingWorkBuf, '\n');
	while (cp1)
	{
		if (cp1[-1] == '\r')
			cp1[-1] = 0;
		*cp1++ = 0;
		m_cpExistingWork[m_nExistingWork++] = cp;
		cp = cp1;
		cp1 = strchr(cp, '\n');
	}
	if (!m_nExistingWork)
	{
		delete[] m_cpExistingWorkBuf;
		m_cpExistingWorkBuf = 0;
	}

	// See if we found any of the "global" missing primes.
	sprintf(FName, "%s/wasmissing.wip", m_pAccum->ClientDir);

	if (!_access(FName, 0))
	{
		// Remove this so we don't handle this next time (which might not have any "missing" data.
		remove(FName);
		rename("missing.wip", "missing.tmp");
		FILE *out = fopen("missing.wip", "wb");
		if (!out)
			rename("missing.tmp", "missing.wip");
		else
		{
			PFSimpleFile *pFile = openInputFile("missing.tmp");
			if (!pFile)
			{
				fclose(out);
				remove("missing.wip");
				rename("missing.tmp", "missing.wip");
			}
			else
			{
				PFString sLine;
				while (!pFile->GetNextLine(sLine))
				{
					unsigned n = 0;
					bool bFound = false;
					while (n < m_nExistingWork)
					{
						if (!strcmp(m_cpExistingWork[n], (const char *)sLine))
						{
							bFound = true;
							break;
						}
						++n;
					}
					if (!bFound)
						fprintf(out, "%s\r\n", (const char*)sLine);
				}
				delete pFile;
				fclose(out);
				remove ("missing.tmp");
			}
		}
	}
}

bool CClientData::ClientStats_End(HANDLE hand, FILE *pr, FILE *comp, FILE *skip)
{
    if (hand != m_hCurrentHandle)
    {
        // WHAT TO DO!?!?!?!?!
#if defined (_DEBUG)
		MessageBox(0, "Error, hand != m_hCurrentHandle", "Error 1", 0);
		DebugBreak();
#endif
    }
	m_nExistingWork = 0;
	if (m_pAccum->m_nComps || m_pAccum->m_nPrimes || m_pAccum->m_nSkips)
		LoadExistingWork();

	uint32 n = 0;
	if (m_pAccum->m_nComps)
	{
		for (uint32 i = 0; i < m_pAccum->m_nComps; ++i)
		{
			while (n < m_nExistingWork)
			{
				if (!strcmp(m_cpExistingWork[n], m_pAccum->m_Comps[i]))
				{
					m_cpExistingWork[n++][0] = 0;
					break;
				}
				++n;
			}
			fputs(m_pAccum->m_Comps[i], comp);
			fputs("\n", comp);
		}
	    fflush(comp);
		_close(_dup(_fileno(comp)));
	}
	n=0;
	if (m_pAccum->m_nSkips)
	{
		for (uint32 i = 0; i < m_pAccum->m_nSkips; ++i)
		{
			while (n < m_nExistingWork)
			{
				if (m_cpExistingWork[n][0] && !strcmp(m_cpExistingWork[n], m_pAccum->m_Skips[i]))
				{
					m_cpExistingWork[n++][0] = 0;
					break;
				}
				++n;
			}
			fputs(m_pAccum->m_Skips[i], skip);
			fputs("\n", skip);
		}
	    fflush(skip);
		_close(_dup(_fileno(skip)));
	}
	n=0;
	if (m_pAccum->m_nPrimes)
	{
		for (uint32 i = 0; i < m_pAccum->m_nPrimes; ++i)
		{
			while (n < m_nExistingWork)
			{
				if (m_cpExistingWork[n][0] && !strcmp(m_cpExistingWork[n], m_pAccum->m_Primes[i]))
				{
					m_cpExistingWork[n++][0] = 0;
					break;
				}
				++n;
			}
			fputs(m_pAccum->m_Primes[i], pr);
			fputs("\n", pr);
		}
	    fflush(pr);
		_close(_dup(_fileno(pr)));
	}

	if (m_nExistingWork)
	{
		FILE *out=0, *out1=0;
		bool bOut=false;
		char FName[256];
		if (m_bLoggingOut)
		{
			sprintf (FName, "%s/current.wip", m_pAccum->ClientDir);
			out = fopen(FName, "wb");
		}
		else
		{
			sprintf (FName, "%s/missing.wip", m_pAccum->ClientDir);
			out = fopen(FName, "ab");
			sprintf (FName, "missing.wip");
			out1 = fopen(FName, "ab");
		}
		for (uint32 i = 0; i < m_nExistingWork; ++i)
		{
			if (m_cpExistingWork[i][0])
			{
				bOut = true;
				fprintf (out, "%s\n", m_cpExistingWork[i]);
				if (out1)
					fprintf (out1, "%s\n", m_cpExistingWork[i]);
			}
		}
		fclose(out);
		if (out1)
			fclose(out1);
		delete[] m_cpExistingWorkBuf;
		m_cpExistingWorkBuf = 0;
		m_nExistingWork = 0;
		if (!m_bLoggingOut || !bOut)
		{
			sprintf (FName, "%s/current.wip", m_pAccum->ClientDir);
			remove(FName);
		}
	}

    ComputeScores();

    return true;
}

void CClientData::ClientStats_Cleanup()
{
	delete m_pAccum;
	m_pAccum = 0;
}

bool CClientData::ClientStats_AddWorkDone(HANDLE hand, const char *cp)
{
    if (hand != m_hCurrentHandle)
    {
#if defined (_DEBUG)
		MessageBox(0, "Error, hand != m_hCUrrentHandle", "Error 2", 0);
		DebugBreak();
#endif
    }
    uint16 z = *(uint16*)&cp[3];
    if (z == '_C') // composite
		m_pAccum->m_Comps[m_pAccum->m_nComps++] = &cp[5];
    else
		m_pAccum->m_Primes[m_pAccum->m_nPrimes++] = &cp[3];

    return true;
}

bool CClientData::ClientStats_AddWorkSkipped(HANDLE hand, const char *Work)
{
    if (hand != m_hCurrentHandle)
    {
#if defined (_DEBUG)
		MessageBox(0, "Error, hand != m_hCUrrentHandle", "Error 3", 0);
		DebugBreak();
#endif
    }
	m_pAccum->m_Skips[m_pAccum->m_nSkips++] = &Work[3];
    return true;
}

bool CClientData::ClientStats_GetNewWork(HANDLE hand, uint32 &Requested, SOCKET s, FILE *in, bool &bCloseWhenDone, bool bShouldCompress, bool bIsFirstLineFile, char *FirstLine, const char *CurInFName)
{
    if (hand != m_hCurrentHandle)
    {
#if defined (_DEBUG)
		MessageBox(0, "Error, hand != m_hCurrentHandle", "Error 4", 0);
		DebugBreak();
#endif
    }

	static char CmprBuf[256*1024+sizeof(tcp_ver2_xfer)], oBuf[256*1024+sizeof(tcp_ver2_xfer)];;

	tcp_ver2_xfer *pXfer = (tcp_ver2_xfer*)oBuf;
	char *ocp = pXfer->cpData;
	*ocp = 0;
	uint32 nNumWanted = Requested;
	Requested = 0;

	if (nNumWanted > 75000)
		nNumWanted = 75000;

	bool bIsABCD=false;
	uint64 ABCDAccum = 0;

	char FName[256];
	sprintf (FName, "%s/missing.wip", m_pAccum->ClientDir);
	FILE *in_missing = fopen(FName, "rb");
	if (in_missing)
	{
		// If they have outstanding work, then FORCE them to finish it
		while (nNumWanted && !feof(in_missing))
		{
			*ocp = 0;
			fgets(ocp, sizeof(oBuf)-sizeof(tcp_ver2_xfer)-(ocp-pXfer->cpData)-1, in_missing);
			if (*ocp)
			{
				ocp += strlen(ocp);
				nNumWanted--;
				//Requested++;
			}
		}
		fclose(in_missing);
		remove(FName);
		sprintf(FName, "%s/wasmissing.wip", m_pAccum->ClientDir);
		FILE *fp = fopen(FName, "wb");
		fclose(fp);
	}
	else
	{
		if (bIsFirstLineFile)
		{
			// NOTE even the ABCD file has the "first" line correct at this point.
			strcpy(ocp, FirstLine);
			ocp += strlen(ocp);
			if (!strncmp(FirstLine, "ABCD ", 5))
			{
				bIsABCD = true;
				char *cp = strchr(FirstLine, '[');
				if (!cp)
					MessageBox(0, "Error", "Error first line is ABCD, but no '[' char found", 0);
				else
				{
					// The "header" line of an ABCD file IS a candidate (unlike NewPGen and ABC files).
					Requested++;
					nNumWanted--;
					++cp;
					ABCDAccum = _atoi64(cp);
				}
			}
		}
		if (bIsABCD)
		{
			while (nNumWanted && !feof(in))
			{
				*ocp = 0;
				fgets(ocp, sizeof(oBuf)-sizeof(tcp_ver2_xfer)-(ocp-pXfer->cpData)-1, in);
				if (*ocp)
				{
					int x = atoi(ocp);
					if (!x)
					{
						if (!strncmp(ocp, "ABCD ", 5))
						{
							strcpy (FirstLine, ocp);
							char *cp = strchr(ocp, '[');
							if (!cp)
								MessageBox(0, "Error", "Error first line is ABCD, but no '[' char found", 0);
							else
							{
								++cp;
								ABCDAccum = _atoi64(cp);
							}
						}
					}
					else
						ABCDAccum += x;
					ocp += strlen(ocp);
					nNumWanted--;
					Requested++;
				}
			}
		}
		else
		{
			while (nNumWanted && !feof(in))
			{
				*ocp = 0;
				fgets(ocp, sizeof(oBuf)-sizeof(tcp_ver2_xfer)-(ocp-pXfer->cpData)-1, in);
				if (*ocp)
				{
					ocp += strlen(ocp);
					nNumWanted--;
					Requested++;
				}
			}
		}
	}


	// write out the "wip" file.
	sprintf (FName, "%s/current.FName", m_pAccum->ClientDir);
	FILE *out = fopen(FName, "wt");
	fprintf (out, "%s\n", CurInFName);
	fclose(out);

	sprintf (FName, "%s/current.wip", m_pAccum->ClientDir);
	if (!bIsFirstLineFile)
	{
		FILE *out = fopen(FName, "wb");
		if (out)
		{
			fputs(pXfer->cpData, out);
			fclose(out);
		}
	}
	else
	{
		// We have to "parse" the file here :((((
		char FName1[256];
		sprintf (FName1, "%s/tmp.wip", m_pAccum->ClientDir);
		FILE *tmp_out = fopen(FName1, "wb");
		if (tmp_out)
		{
			fputs(pXfer->cpData, tmp_out);
			fclose(tmp_out);

			// Now open the file, and read all lines from it (storing those lines into current.wip)
			PFSimpleFile *pFile = openInputFile(FName1);
			if (pFile)
			{
				FILE *out = fopen(FName, "wb");
				if (out)
				{
					PFString sLine;
					while (!pFile->GetNextLine(sLine))
					{
						fprintf (out, "%s\r\n", (const char*)sLine);
					}
					fclose(out);
				}
				delete pFile;
			}
		}
		remove(FName1);

		// For now, simply "ignore" the missing stuff for "Line1" files.
	}

	if (bIsABCD)
	{
		// recompute the FirstLine for the next caller.
		char *cp = strchr(FirstLine, '[');
		*cp = 0;
		char NewLine[4096];
		fgets(NewLine, sizeof(NewLine), in);
		if (!strncmp(NewLine, "ABCD ", 5))
			strcpy(FirstLine, NewLine);
		else
		{
			cp++;
			cp = strchr(cp, ']');
			if (!cp)
				MessageBox(0, "Error", "Error first line is ABCD, but no ']' char found", 0);
			else
			{
				++cp;
				ABCDAccum += atoi(NewLine);
				sprintf (NewLine, "%s[%I64d]%s", FirstLine, ABCDAccum, cp);
				strcpy(FirstLine, NewLine);
			}
		}

	}

	if (feof(in))
	{
		fclose(in);
		bCloseWhenDone = true;
	}
	// Ok, now we have the beast, so lets update it correctly and send it.
	pXfer->u32UnpackedLen = pXfer->u32PackedLen = strlen(pXfer->cpData)+1;
	pXfer->eCompressed = e_COMPRESSION_NONE;

	// Ok, now try to comress it (if we are told to do so)
	if (bShouldCompress)
	{
		uint32 len = 256*1024-100;
		if (PFGW_deflate((uint8*)CmprBuf, (uint8*)pXfer->cpData, &len, pXfer->u32UnpackedLen) && len < 256*1024)
		{
			memcpy(pXfer->cpData, CmprBuf, len);
			pXfer->u32PackedLen = len;
			pXfer->eCompressed = e_COMPRESSION_ZLIB;
		}
	}
	send(s, oBuf, pXfer->u32PackedLen-1+sizeof(tcp_ver2_xfer), 0);

    return true;
}

bool CClientData::ClientStats_LogIn(HANDLE hand, const char *Name)
{
    if (hand != m_hCurrentHandle)
    {
#if defined (_DEBUG)
		MessageBox(0, "Error, hand != m_hCurrentHandle", "Error 5", 0);
		DebugBreak();
#endif
    }
    return true;
}

bool CClientData::ClientStats_LogOut(HANDLE hand, const char *Name)
{
    if (hand != m_hCurrentHandle)
    {
#if defined (_DEBUG)
		MessageBox(0, "Error, hand != m_hCurrentHandle", "Error 6", 0);
		DebugBreak();
#endif
    }
	m_bLoggingOut = true;
    return true;
}

bool CClientData::ClientStats_Operation(HANDLE hand, const char *cp)
{
    if (hand != m_hCurrentHandle)
    {
#if defined (_DEBUG)
		MessageBox(0, "Error, hand != m_hCUrrentHandle", "Error 7", 0);
		DebugBreak();
#endif
    }
    return true;
}

const char*CClientData::ClientStats_CurInFileName()
{
	return m_pAccum->ClientFName;
}
