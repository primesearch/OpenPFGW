#ifdef USE_NETWORK

#include "pfiopch.h"
#include <string.h>
#include <stdio.h>
#include "pfnetworkfile.h"

PFNetworkFile::PFNetworkFile(const char* FileName)
	: PFSimpleFile(FileName), curExpr(0), maxExpr(0)
{
	cli=new tcpip_client();
}

PFNetworkFile::~PFNetworkFile()
{
	char str[NETLINELEN*MAXEXPRS];
	strcpy(str,"");
	for (int i=curExpr;i<maxExpr;i++) {
		strcat(str,"ND ");
		strcat(str,expr[i]);
		strcat(str,"\n");
	}
	cli->Send(str);
	Sleep(100);
	delete cli;
}

int PFNetworkFile::SeekToLine(int /*LineNumber*/) { return e_ok; }

int PFNetworkFile::GetNextLine(PFString &sLine, Integer * /*num*/, bool *b, PFSymbolTable *)
{
	char *ptr1=NULL, *ptr2, sendStr[80];
	int count=0,i;

	if (b)
		*b=false;
	
/*	if (m_bIsBeingResumed) {
		sLine=*m_pIni->GetExpr();
		m_sCurrentExpression=sLine;
		m_bIsBeingResumed=false;
		return e_ok; // !!
	}*/

	if (curExpr>=maxExpr) {
		sprintf(sendStr,"GN %d\n",int(ExprsToGet));

		if (cli->Send(sendStr)!=0)
			return e_eof;
		ptr1=cli->Recv(30);
		
		while (ptr1==NULL && count<30) {
			if (cli->Error != WSAEWOULDBLOCK) 
				return e_eof;
			if ((count&3)==3 && cli->Send(sendStr)!=0)
				return e_eof;
			ptr1=cli->Recv(30);
			count++;
		}
		
		if (ptr1==NULL)
			return e_eof;
		maxExpr=ExprsToGet;
		for (i=0;i<ExprsToGet;i++) {
			ptr2=strchr(ptr1,'\n');
			if (ptr2!=NULL) {
				*ptr2=0;
				strcpy(expr[i],ptr1);
				strtok(expr[i],"\n\r");
				ptr1=ptr2+1;
			} else {
				maxExpr=i;
				break;
			}
		}
		if (maxExpr==0)
			return e_eof;
		curExpr=0;
	}
	sLine=expr[curExpr];
	curExpr++;
	m_sCurrentExpression=sLine;
	return e_ok;
}

void PFNetworkFile::LoadFirstLine()
{
    unsigned short port;
	
	PFPrintf("Recognized Network file: ");

	char Line[128],Line2[10],Line3[10];
	if (ReadLine(Line, sizeof(Line)))
	{
		fclose(m_fpInputFile);
		throw "Not a valid file";
	}

	if (ReadLine(Line,sizeof(Line)))
	{
		fclose(m_fpInputFile);
		throw "Not a valid network file";
	}

	if (ReadLine(Line2,sizeof(Line2)))
	{
		fclose(m_fpInputFile);
		throw "Not a valid network file";
	}

	if (ReadLine(Line3,sizeof(Line3)))
	{
		ExprsToGet=atoi(Line3);
		if (ExprsToGet>MAXEXPRS)
			ExprsToGet=MAXEXPRS;
	} else {
		ExprsToGet=5;
	}

	port=(unsigned short)atoi(Line2);

	if (cli->Connect(Line,port))
	{
		fclose(m_fpInputFile);
		throw "Can't find server";
	}
	m_nCurrentLineNum=0;  //And it always will be!!
}

void PFNetworkFile::CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid, PFString * /*p_MessageString*/)
{
	char str[NETLINELEN];

	if (p_bMessageStringIsValid)
		*p_bMessageStringIsValid = false;

	if (!bIsPrime && !bIsPRP)
		return;
	
	strcpy(str,"PR ");
	strcat(str,LPCTSTR(m_sCurrentExpression));
	strcat(str,"\n");

	cli->Send(str);
}

#endif
