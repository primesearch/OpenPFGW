#ifdef USE_NETWORK

#include "pfiopch.h"
#include <string.h>
#include <stdio.h>
#include <io.h>
#include "pfnetworkfile2.h"

extern bool volatile g_bExitNow;
extern uint64 g_u64ResidueVal;

#define NET2_DONEFILE "NetWork2.don"
#define NET2_WIPFILE  "NetWork2.wip"
#define NET2_WIPFILE2 "NetWork2_.wip"

PFNetworkFile2::PFNetworkFile2(const char* FileName)
   : PFSimpleFile(FileName)
{
	maxExpr=ExprsToGet=0;
	m_nCurrentLineNum = 0x7FFFFFFF;
	m_bVerboseMode=false;
	m_bAbortMode = false;

	m_SendStr = new char [256*1024];
	memset(m_SendStr, 0, 256*1024);
//	m_WaitTil = GetTickCount() + 30*60*1000;  // Only send data once every 30 minutes, or when out of data
	m_bSendCompositesAlso = false;
	m_bSendResidues = false;

	// We need to load this file and the other data, and resume if restarting.
	m_fpSendStr = 0;
	m_fpSendStr = fopen(NET2_DONEFILE, "r+t");
	if (m_fpSendStr)
		// read the data into the m_SendStr data element, and move the file pointer to the end of 
		// the file (new data will be added to the end of the file).
		fread(m_SendStr, 1, _filelength(_fileno(m_fpSendStr)), m_fpSendStr);
	else
		m_fpSendStr = fopen(NET2_DONEFILE, "wt");

	const char *cpErr;
	pf = openInputFile(NET2_WIPFILE, NULL, &cpErr);
	if (pf)
	{
		// find the "last" expression in the m_fpSendStr
		char *cp1 = strrchr(m_SendStr, ' ');
		char *cp2 = strrchr(m_SendStr, '_');
		char *cpLast = cp1;
		if (!cpLast)
		{
			if (!cp2)
				cpLast = " ";
			else
				cpLast = cp2;
		}
		else if (cp2 && cp2 > cpLast)
			cpLast = cp2;
		++cpLast;
		char *cpCRLF = strchr(cpLast, '\n');
		if (cpCRLF)
			*cpCRLF = 0;
		if (cpLast && strlen(cpLast))
		{
			PFString sLine;
			m_nCurrentLineNum = 0;
			bool bFnd = false;
			while (pf->GetNextLine(sLine) != e_eof && !bFnd)
			{
				if (!strcmp(cpLast, sLine))
					bFnd = true;
				m_nCurrentLineNum++;
			}
			if(cpCRLF)
				*cpCRLF = '\n';
		}
	}
}

PFNetworkFile2::~PFNetworkFile2()
{
	unsigned nLen = 256*1024, i;
	char *str = new char [nLen];
	*str = 0;
	char *cp = str;
	PFString Str;

	if (m_bAbortMode && maxExpr)
	{
		// Only send data back to the server, IF we have some.  Otherwise don't send
		// anything back.
		i=m_pIni->GetFileLineNum()-1;
		if ((int)i < 0)
			i = 0;
		for (;i<maxExpr;++i)
		{
			pf->GetNextLine(Str);
			if ( (cp-str) + 3 + 1 + 1 + strlen(Str) > nLen)
			{
				nLen += 256*1024;
				char *p = new char[nLen];
				strcpy(p, str);
				int len = cp-str;
				delete[] str;
				str = p;
				cp = &str[len];
			}
			cp += sprintf(cp, "ND %s\n", (const char *)Str);
		}
		// We don't keep a WIP file since we are aborting.
		remove(NET2_WIPFILE);
	}

	// Close the work done file.  We ONLY delete this file if the data gets sent to the server.
	fclose(m_fpSendStr);

	int Err;
	Err = cli->Connect(Host, Port);
	if (Err)
		PFPrintf ("Error connecting to server to send the DID NOT DO results.  Return %d Error %d\n", Err, cli->Error);
	else
	{
		// Now also send the done information.
		strcat(m_SendStr, str);

		// Log out from server
		strcat(m_SendStr, "LO ");
		strcat(m_SendStr, m_szWhoAmI);
		strcat(m_SendStr, "\n");


		if (cli->Send(m_SendStr)!=0)
		{
			Err = cli->Error;
			PFPrintf ("Error sending PRP data to server.  Return %d Error %d ", Err, cli->Error);
		}
		else
		{
			// we have sent it, so delete the cached file from disk.
			remove(NET2_DONEFILE);
			// just in case.  We give the connection just a little bit of time to send data.

			Str="";
			if (pf)
			{
				pf->GetNextLine(Str);
				if (Str != "")
				{
					FILE *fp_Wip = fopen(NET2_WIPFILE2, "wt");
					while (Str != "")
					{
						fprintf(fp_Wip, "%s\n", (const char *)Str);
						Str = "";
						pf->GetNextLine(Str);
					}
					fclose(fp_Wip);
					delete pf;
					pf = 0;
					remove(NET2_WIPFILE);
					rename(NET2_WIPFILE2, NET2_WIPFILE);
					Sleep(250);
				}
			}
		}
	}
	delete cli;
	delete[] str;
	delete[] m_SendStr;
	delete pf;
}

int PFNetworkFile2::SeekToLine(int /*LineNumber*/)
{
	// Save data here to a local file and to the .ini file so we can restart.
	return e_ok;
}

bool PFNetworkFile2::ConnectToServer()
{
	if (g_bExitNow)
		return false;
	int Err;
#if defined (_DEBUG)
	if (m_bVerboseMode)
		PFPrintf ("Connecting to host  ");
#endif

	while ( (Err=cli->Connect(Host, Port)) != 0)
	{
		PFPrintf ("Trying to connect to server.  Return %d Error %d\n", Err, cli->Error);
		// Wait 30 seconds, and try again.  Try until the user exits.
		for (int Cnt = 0; Cnt < 300; Cnt++)
		{
			Sleep(100);
			if (g_bExitNow)
				return false;
		}
		// At this point, we MUST try to use the EMAIL gimme your IP method to get the IP from the server. That will
		// be done at some later point when we get the email version working.  For now, just retry to same address
	}

#if defined (_DEBUG)
	if (m_bVerboseMode)
		PFPrintf ("Connected  ");
#endif

	return Err == 0;
}


bool PFNetworkFile2::SendRequestToServer(char *Str)
{
	if (*Str)
	{

#if defined (_DEBUG)
		if (m_bVerboseMode)
			PFPrintf ("Sending Data ");
#endif
		if (cli->Send(Str)!=0)
		{
			int Err = cli->Error;
			cli->Close();
			if (Err == 10054)
			{
				// 10054 An existing connection was forcibly closed by the remote host. 
				if (m_bVerboseMode)
					PFPrintf ("Error sending PRP to server.  Return %d Error %d ", Err, cli->Error);
				return TryAgain_in5seconds();
			}
			PFPrintf ("Error sending PRP to server.  Return %d Error %d ", Err, cli->Error);
			return TryAgain_in60seconds();
		}
	}
	return true;
}
bool PFNetworkFile2::SendResultsToServer()
{
	return true;
}

bool PFNetworkFile2::GetResponseFromServer()
{
	return true;
}

bool PFNetworkFile2::LogoutOfServer()
{
	return true;
}

int PFNetworkFile2::GetNextLine(PFString &sLine, Integer *num, bool *b)
{
	char sendStr[80];
	tcp_ver2_xfer *pRecvPtr;

	if (b)
		*b=false;

ReadAgain:;

	int EoF;
	if (!pf)
		EoF = PFSimpleFile::e_eof;
	else
		EoF = pf->GetNextLine(sLine, num, b);
	if (EoF == PFSimpleFile::e_eof)
	{
		delete pf;
		pf = 0;
		remove(NET2_WIPFILE);

		// Start at the beginning of the list again.
		m_nCurrentLineNum = 0;

		int GetThisTime = ExprsToGet-maxExpr;

TryAgain:;

		if (!ConnectToServer())
			return e_eof;	// the only way this will fail is if we can't connect, and the user clicks EXIT.

		//sprintf(sendStr,"PN %d\n",int(GetThisTime));	// Prep the server to tell it how many to "prepare" for us.
		//if (!SendRequestToServer(sendStr))
		//	goto TryAgain;

		if (*m_SendStr)
		{
			if (!SendRequestToServer(m_SendStr))
				goto TryAgain;
			*m_SendStr = 0;
			// Since the data was sent, we need to truncate the file (we are "starting over");
			fclose(m_fpSendStr);
			m_fpSendStr = fopen(NET2_DONEFILE, "wt");

			cli->Close();
			Sleep(250);
			goto TryAgain;
		}

		if (m_bVerboseMode)
			PFPrintf ("Getting %d expressions ", GetThisTime);
		sprintf(sendStr,"GN %d\n",int(GetThisTime));
		if (!SendRequestToServer(sendStr))
			goto TryAgain;

		pRecvPtr=cli->Recv();

		if (pRecvPtr==NULL)
		{
			PFPrintf (" No work from server at this time, ");
			TryAgain_in60seconds();
			goto TryAgain;
		}

#if defined (_DEBUG)
		// only here to "test" with 
		/*
		FILE *out = fopen("data.data", "ab");
		fwrite(pRecvPtr->cpData, 1, pRecvPtr->dwPackedLen, out);
		fclose (out);
		*/
#endif

		if (!strcmp(pRecvPtr->cpData, "NoData\n"))
		{
			free(pRecvPtr);
			
			TryAgain_in60seconds();
			goto TryAgain;
		}
		// Should we use a "wt" or "at"?  I think an "wt" is right, but I have left the "at" for now.
		// NOTE it did take an "wt" to work correctly, but I added the remove() above, to delete the 
		// file when the server has no more data.
		FILE *fp_Wip = fopen(NET2_WIPFILE, "wt");

		fwrite(pRecvPtr->cpData, 1, pRecvPtr->u32UnpackedLen, fp_Wip);
		fclose(fp_Wip);
		free(pRecvPtr);

		cli->Close();

		const char *cpErr;
		pf = openInputFile(NET2_WIPFILE, NULL, &cpErr);

		goto ReadAgain;
	}
	m_sCurrentExpression=sLine;
	m_nCurrentLineNum++;

	// Might as well let WinPFGW show how much work this client has done.
	if (m_pIni)
		m_pIni->SetFileLineNum(m_nCurrentLineNum);
	return e_ok;
}

bool PFNetworkFile2::TryAgain_in60seconds()
{
	if (cli)
	cli->Close();
#if defined (_DEBUG)
	PFPrintf ("will retry in ~5 seconds\n");
#else
	PFPrintf ("will retry in %d minutes\n", NoDataTimeoutMin);
#endif
	// Wait 60 seconds, and try again.  Try until the user exits.
	int Cnt;
#if defined (_DEBUG)
	for (Cnt = 0; Cnt < 50; Cnt++)
#else
	for (Cnt = 0; Cnt < NoDataTimeoutMin*600+1; Cnt++)
#endif
	{
		Sleep(100);
		if (g_bExitNow)
			return false;
	}
	return false;
}

bool PFNetworkFile2::TryAgain_in5seconds()
{
	if (cli)
		cli->Close();
	if (m_bVerboseMode)
		PFPrintf ("will retry in ~5 seconds\n");
	// Wait 5 seconds, and try again.  Try until the user exits.
	srand((unsigned int) time(0));
	for (int Cnt = 0; Cnt < 50; Cnt++)
	{
		int x = rand();
		Sleep( (x%47 + x%53) << 1) ;
		if (g_bExitNow)
			return false;
	}
	return false;
}

void PFNetworkFile2::LoadFirstLine()
{
	PFPrintf("Recognized Network2 file: ");

	// First things first, load the client name, then create teh TCP object.
	PFString sWhoAmI, sKey("Netork2WhoAmIName"), sDefault("PFGW_Client");
	m_pIni->GetIniString(&sWhoAmI, &sKey, &sDefault, true);

	// Then load the "timeout" value for when the server has no data
	sKey = "Network2NoDataTimeoutWaitMinutes";
	m_pIni->GetIniInt(&NoDataTimeoutMin, &sKey, 5, true);

	strncpy(m_szWhoAmI, (LPCSTR)sWhoAmI, 39);
	m_szWhoAmI[39] = 0;

	cli=new tcpip_client2(m_szWhoAmI);


	char Line[128],Line2[10],Line3[10];
	if (ReadLine(Line, sizeof(Line)))
	{
		fclose(m_fpInputFile);
		throw "Not a valid file";
	}
	PFPrintf ("Line is %s\n", Line);

	if (ReadLine(Host,sizeof(Host)))
	{
		fclose(m_fpInputFile);
		throw "Not a valid network file, No HOST line";
	}
	// Win95's Winsock was puking (not WinSock2), when the \n char was part of the Host name.  Simple to remove it.
	strtok(Host, "\n\r");

	if (ReadLine(Line2,sizeof(Line2)))
	{
		fclose(m_fpInputFile);
		throw "Not a valid network file, No PORT line";
	}

	if (ReadLine(Line3,sizeof(Line3)))
	{
		PFPrintf ("Line3 failed\n");
		ExprsToGet=5;
	}
	else
	{
		if (strstr(Line3, ",v") || strstr(Line3, ",V"))
			m_bVerboseMode = true;
		if (strstr(Line3, ",c") || strstr(Line3, ",C"))
			m_bSendCompositesAlso = true;
		if (strstr(Line3, ",r") || strstr(Line3, ",R"))
			m_bSendResidues = true;
		if (strstr(Line3, ",a") || strstr(Line3, ",A"))
			m_bAbortMode = true;
		ExprsToGet=atoi(Line3);
	}

	Port= (unsigned short)atoi(Line2);

	// At this point, we should NOT loop if there is work to do (if the host can't be reached).
	while (cli->Connect(Host,Port))
	{
		if(m_nCurrentLineNum != 0x7FFFFFFF)
		{
			PFPrintf ("login connect failed to server at %s : %d\n", Host, Port);
			PFPrintf ("However, there is still work left. Proceeding with that\n");
			return;
		}
		PFPrintf ("login connect failed to server at %s : %d\nTry Again in 5 minutes\n", Host, Port);
		for (int j = 0; j < 60*5 && !g_bExitNow; ++j)
			Sleep(1000);
		if (g_bExitNow)
		{
			fclose(m_fpInputFile);
			throw "Exiting, can't contact server";
		}
	}
	if (m_nCurrentLineNum == 0x7FFFFFFF)
	{
		m_nCurrentLineNum = maxExpr;
	}
	if (m_pIni)
		m_pIni->SetFileLineNum(m_nCurrentLineNum);


	char MsgBuf[120];

	// Logs in.  Send LI (log in) and name, and sends OP messages of Compression is OK to use,
	// and Protocol v2.01 is understood.
	if (cli->bCompressionOK())
		sprintf (MsgBuf, "LI %s\nOP CompressionOK\nOP Proto_v_2_01\n", m_szWhoAmI);
	else
		sprintf (MsgBuf, "LI %s\nOP NOCompression\nOP Proto_v_2_01\n", m_szWhoAmI);

	cli->Send(MsgBuf,true);	// NEVER compress this message.  
	cli->Close();
}


char OutBuf[50000];

void PFNetworkFile2::CurrentNumberIsPrime(bool bIsPrime, bool *p_bMessageStringIsValid, PFString * /*p_MsgStr*/)
{
	if (p_bMessageStringIsValid)
		*p_bMessageStringIsValid = false;

	char Buf[5000];
	*Buf = 0;
	int nBufLen;

	// Caching added, so that writes only happen at most once every 30 seconds.
	static int nOutBuf;
	static DWORD dwLastSend;

	// ALSO store to a local file, so that a stop/restart will still have this data.
	if (!bIsPrime)
	{
		if (!m_bSendCompositesAlso)
			// Note this will cause the resuming to "fail" and possibly reprocess composites
			return;
		if (m_bSendResidues)
			nBufLen = sprintf(Buf, "PR C_%s [%lX%08lX]\n", LPCTSTR(m_sCurrentExpression), 
							 (uint32)(g_u64ResidueVal>>32), (uint32)(g_u64ResidueVal&0xFFFFFFFF));
		else
			nBufLen = sprintf (Buf, "PR C_%s\n", LPCTSTR(m_sCurrentExpression));
	}
	else
		nBufLen = sprintf (Buf, "PR %s\n", LPCTSTR(m_sCurrentExpression));

	// NOTE this indicates that the data has just been sent to the server, so we need to "clean" up our
	// cached data
	if (m_SendStr[0] == 0)
	{
		nOutBuf = 0;
		*OutBuf = 0;
		dwLastSend = GetTickCount();
	}

	strcat(m_SendStr,Buf);

	// store this information to the "done-data" file and be SURE the file gets correctly
	// (but quickly) hard flushed
	if (nBufLen + nOutBuf > sizeof(OutBuf))
	{
		// write this no matter what
		fprintf(m_fpSendStr, "%s", OutBuf);
		*OutBuf = 0;
		nOutBuf = 0;
		fflush(m_fpSendStr);
		_close(_dup(_fileno(m_fpSendStr)));
		dwLastSend = GetTickCount()+ 30000;
	}
	strcpy(&OutBuf[nOutBuf], Buf);
	nOutBuf += nBufLen;
	if (dwLastSend < GetTickCount())
	{
		fprintf(m_fpSendStr, "%s", Buf);
		fflush(m_fpSendStr);
		_close(_dup(_fileno(m_fpSendStr)));
		*OutBuf = 0;
		nOutBuf = 0;
		dwLastSend = GetTickCount() + 30000;
	}

	SendIfTimedOut();
}

void PFNetworkFile2::SendIfTimedOut()
{
//	if (GetTickCount() < m_WaitTil)
		return;

TryAgain:;

	int Err;
	while ( (Err=cli->Connect(Host, Port)) != 0)
	{
		PFPrintf ("Trying to connect to host to send PRP.  Return %d Error %d\n", Err, cli->Error);
		// Wait 5 seconds, and try again.  Try until the user exits.
		for (int Cnt = 0; Cnt < 50; Cnt++)
		{
			Sleep(100);
			if (g_bExitNow)
				return;
		}
	}
	if (cli->Send(m_SendStr)!=0)
	{
		PFPrintf ("Error sending PRP to server.  Return %d Error %d\n", Err, cli->Error);
		cli->Close();
		// Wait 5 seconds, and try again.  Try until the user exits.
		for (int Cnt = 0; Cnt < 50; Cnt++)
		{
			Sleep(100);
			if (g_bExitNow)
				return;
		}
		goto TryAgain;
	}
	*m_SendStr = 0;
	// Since the data was sent, we need to truncate the file (we are "starting over");
	fclose(m_fpSendStr);
	m_fpSendStr = fopen(NET2_DONEFILE, "wt");

	m_WaitTil = GetTickCount() + 30*60*1000;  // Only send data once every 30 minutes
	Sleep(500); // sleep a little before closing the socket
	cli->Close();
}

#endif
