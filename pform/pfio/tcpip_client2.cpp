#ifdef USE_NETWORK

#include <stdlib.h>
#include <string.h>

#include "pfiopch.h"
#include "tcpip_client2.h"

#include "pfgw_zlib.h"

int     tcpip_client2::instances = 0;
bool    tcpip_client2::isInited=false;
WSADATA tcpip_client2::wsaData;

tcpip_client2::tcpip_client2(const char *_ClientName)
{
	bTalkedToServer=false;
	nServerVersion=0;
	m_bCompressionOK=false;
	m_bCompositesOk=false;
	isConnected=false;
	conn_socket=0;
	if (_ClientName)
		strncpy(ClientName, _ClientName, sizeof(ClientName)-1);
	else
		strcpy(ClientName, "PFGWClient");
	ClientName[sizeof(ClientName)-1] = 0;

	if (isInited==false && WSAStartup(0x101,&wsaData) !=0) 
	{
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr("Error %d starting up WinSock!\n", WSAGetLastError());
		isValid=false;
	}
	else
	{
		isValid=true;
		isInited=true;
	}

	m_bCompressionOK = bLoad_zLibDLL();

	instances++;
}

tcpip_client2::~tcpip_client2()
{
	if (isConnected)
	{
		Close();
		if (Error == -3)
			PFPrintf("in tcpip_client2::~tcpip_client2, Close read data Error code%d\n", Error);
	}

	
	if (instances==1 && isInited==true)
	{
		WSACleanup();
		isInited=false;
	}
	instances--;
	Free_zLibDLL();
}

struct hostent * tcpip_client2::GetHost(char *host)
{
	struct hostent *hp;
	unsigned int addr;

	if (isalpha(host[0]))	/* server address is a name */
		hp = gethostbyname(host);
	else					/* Convert nnn.nnn address to a usable one */
	{
		addr = inet_addr(host);
		hp = gethostbyaddr((char *)&addr,4,AF_INET);
		if (!hp)
		{
			//  Ok, somthing bogus (I am seeing this on some of my workstations when things got "corrupted");
			//  Thsi all ONLY uses these things in the hp, so give it to them.
			static struct hostent _hp;
			static unsigned _addr[2] = {addr,0};
			static char *__addr = (char*) &_addr;
			_hp.h_length = 4;
			_hp.h_addr_list = &__addr;
			_hp.h_addrtype = 2;  // TCPIP I think
			return &_hp;
		}
	}
	return hp;
}

#if !defined (SD_SEND)
// NOTE this is defined in Winsock2.h even though it is used in a Winsock 1.1 function
// PFGW client does NOT require Winsock2 to operate, so I don't want to include winsock2.h
// header.  That being the case, I will define the constant here.
#define SD_SEND 1
#endif

void tcpip_client2::Close()
{
	Error=0;
	if (isConnected)
	{
		Sleep(100);
		shutdown(conn_socket, SD_SEND);
		for (;;)
		{
			char cp[256];
			int retval=recv(conn_socket,cp,sizeof(cp),0);
			if (retval==SOCKET_ERROR || retval == 0)
				break;  // all data has been read.
			PFPrintf("in tcpip_client2::Close, We got %d bytes of data when we wanted to close!\n", retval);
			Error=-3;
		}
		closesocket(conn_socket);
	}
	conn_socket=0;
	isConnected=false;
}

int tcpip_client2::Connect(char *host,unsigned short remoteport)
{
	struct hostent *hp;
	struct sockaddr_in server;

	Error=0;
	if (isConnected==true)
		return 0;

	conn_socket = 0;

	try
	{
		if (remoteport==0)
		{
			PFPrintf("in tcpip_client2::Connect, Remote port is zero, so failing!\n", Error);
			return -1;
		}
		hp=GetHost(host);

		if (hp == NULL ) 
		{
			Error=WSAGetLastError();
			PFPrintf("in tcpip_client2::Connect, GetHost() failed with error code %d\n", Error);
			return -2;
		}

		//
		// Copy the resolved information into the sockaddr_in structure
		//
		memset(&server,0,sizeof(server));
		memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
		server.sin_family = hp->h_addrtype;
		server.sin_port = htons(remoteport);
		
		conn_socket = socket(AF_INET,SOCK_STREAM,0); /* Open a socket */
		if (conn_socket <0 )
		{
			PFPrintf("in tcpip_client2::Connect, socket() failed with error code %d  (socket=%d)\n", WSAGetLastError(), conn_socket);
			return -3;
		}

		if (connect(conn_socket,(struct sockaddr*)&server,sizeof(server)) != 0)
		{
			Error=WSAGetLastError();
			PFPrintf("in tcpip_client2::Connect, connect() failed with error code %d\n", Error);
			isConnected = true;
			Close();
			if (Error == -3)
				PFPrintf("in tcpip_client2::Connect, Close (#1) read data Error code%d\n", Error);

			return -4;
		}
		isConnected=true;

		DWORD dwSize = 256*1024;
		setsockopt(conn_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&dwSize, sizeof(DWORD));
		setsockopt(conn_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&dwSize, sizeof(DWORD));

		Error=0;
	}
	catch(...)
	{
		if(conn_socket)
		{
			isConnected = true;
			Close();
			if (Error == -3)
				PFPrintf("in tcpip_client2::Connect, Close (#2) read data Error code%d\n", Error);
		}
		return -5;
	}
	return 0;
}

char CmdBuffer[256*1024];	// covers many smaller xmitts without allocation;
uint8 CompressBuffer[256*1024];

int tcpip_client2::Send(char *str, bool bFORCE_NO_Compress/*=false*/)
{
	if (isConnected==false)
	{
		Error=-2;
		return -2;
	}

	tcp_ver2_xfer *p;
	bool bAllocated=false;

	int sLen = strlen(str);
	// Note sLen CAN NOT be longer than 256k, Not even with compression to work.
	if (sLen - sizeof(tcp_ver2_xfer) < sizeof(CmdBuffer))
		p = (tcp_ver2_xfer *)CmdBuffer;
	else
	{
		p = (tcp_ver2_xfer *)malloc(sLen+sizeof(tcp_ver2_xfer));
		bAllocated = true;
	}

	memset(p, 0, sizeof(tcp_ver2_xfer));
	strcpy(p->szMachineName, ClientName);
	p->u32UnpackedLen = p->u32PackedLen = sLen+1;
	strcpy(p->cpData, str);

	// Compression turned off by default. Only turned on if the DLL is found, and if not in forced-no-compression mode.
	p->eRCompress = e_COMPRESSION_NONE;
	p->eCompressed = e_COMPRESSION_NONE;

	if (m_bCompressionOK && !bFORCE_NO_Compress)
	{
		uint32 comprLen=256*1024-100;	// Max size allowed
		if (PFGW_deflate(CompressBuffer, (uint8*)p->cpData, &comprLen, p->u32UnpackedLen))
		{
			p->eCompressed = e_COMPRESSION_ZLIB;
			p->u32PackedLen = comprLen;
			memcpy(p->cpData, CompressBuffer, comprLen);
		}
		// Tell the server it is ok to send compressed data
		p->eRCompress = e_COMPRESSION_ZLIB;
	}

	if (p->u32PackedLen > 256*1024)
	{
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr("ERROR, Trying to send too many bytes to server.  Max allowed is %d, and we are trying to send %d\n", 24*1024, p->u32PackedLen);
	}
	char *cp = (char*)p;
	DWORD dwToSend = p->u32PackedLen+sizeof(tcp_ver2_xfer)-1;
	while (dwToSend)
	{
		const char *ccp = (const char *)cp;
		int Len = send(conn_socket, ccp, dwToSend, 0);
		if (Len == SOCKET_ERROR)
		{
			// we MUST save this data to disk, and resend on this data as soon as we can!!! but for now bail.
			// a correct version will save and resend at a later time.
			Error=WSAGetLastError();
			if (bAllocated)
				free(p);
			return -1;
		}
		cp += Len;
		dwToSend -= Len;
	}
	Error=0;
	if (bAllocated)
		free(p);
	return 0;
}

// IFF this function returns a pointer, then the caller MUST free() the pointer (not delete but free() )
tcp_ver2_xfer * tcpip_client2::Recv()
{
	int retval;

	tcp_ver2_xfer xBase;

	if (isConnected==false)
	{
		Error=-2;
		return NULL;
	}
	char *cp = (char*)&xBase;
	memset(cp, 0, sizeof(xBase));
	int NeededLen = sizeof(xBase)-1;
	while (NeededLen)
	{
		retval=recv(conn_socket,cp,NeededLen,0);
		if (retval==SOCKET_ERROR)
		{
			Error=WSAGetLastError();
			return NULL;
		}
		NeededLen -= retval;
		cp += retval;
	}
	// Ok, we got the tcp_ver2_xfer header.  Now find out how many bytes to read, and get them.

	// max xmit size is only 256k.  If something trys to xmit more than this, then the server is 
	// broken.
	if (xBase.u32PackedLen > 256*1024)
		return NULL;
	char *Buf = (char*)malloc(sizeof(tcp_ver2_xfer)+xBase.u32PackedLen);
	memcpy(Buf, &xBase, sizeof(tcp_ver2_xfer));
	tcp_ver2_xfer  *pXfer = (tcp_ver2_xfer *)Buf;
	cp = pXfer->cpData;
	int BytesToGet = xBase.u32PackedLen;
	while (BytesToGet)
	{
		retval=recv(conn_socket,cp,BytesToGet, 0);
		if (retval==SOCKET_ERROR)
		{
			PFPrintf("Socket Error %d arrived after getting %d bytes of %d needed of data", WSAGetLastError(), xBase.u32PackedLen-BytesToGet, xBase.u32PackedLen);
			pXfer->cpData[xBase.u32PackedLen-BytesToGet] = 0;  // null terminate (since we have a broken packet.
			return pXfer;
		}
		if (retval==0)
		{
			PFPrintf("Socket closed out on us %d arrived after getting %d bytes of %d needed of data", WSAGetLastError(), xBase.u32PackedLen-BytesToGet, xBase.u32PackedLen);
			pXfer->cpData[xBase.u32PackedLen-BytesToGet] = 0;  // null terminate (since we have a broken packet.
			return pXfer;
		}
		BytesToGet -= retval;
		cp += retval;
	}
	// Fully Successful!!

	if (pXfer->eCompressed == e_COMPRESSION_ZLIB)
	{
		if (!m_bCompressionOK)
		{
			// Damn, we don't have the DLL and the server sent us data compressed (it should NOT do that)
			PFPrintf("Error, server replied to us with compressed data & we can't handle it!! Find the zLib.dll and restart PFGW\n");
			free(Buf);
			return NULL;
		}

		// Ok, now decompress the damn thing.
		char *Buf2 = (char*)malloc(sizeof(tcp_ver2_xfer)+pXfer->u32UnpackedLen+10);
		tcp_ver2_xfer  *pXfer2 = (tcp_ver2_xfer *)Buf2;
		memcpy(Buf2, Buf, sizeof(tcp_ver2_xfer));
		pXfer2->u32UnpackedLen += 10;
		PFGW_inflate((uint8*)pXfer2->cpData, (uint8*)pXfer->cpData, pXfer->u32PackedLen, &pXfer2->u32UnpackedLen);
		if (pXfer2->u32UnpackedLen != pXfer->u32UnpackedLen)
		{
			PFOutput::EnableOneLineForceScreenOutput();
			PFPrintfStderr ("Error, should have decompressed to %d bytes, not %d bytes\n", pXfer->u32UnpackedLen, pXfer2->u32UnpackedLen);
		}
		pXfer2->u32PackedLen = pXfer2->u32UnpackedLen;
		delete[] Buf;
		pXfer = pXfer2;
	}
	return pXfer;
}

#endif
