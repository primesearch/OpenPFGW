#if !defined (__TCPIP_Client2_H__)
#define __TCPIP_Client2_H__

#include <winsock.h>

/* Your include-file code ... */

#include "tcp_ver2_xfer.h"

class tcpip_client2 {
	static struct hostent * GetHost(char *host);
	static int instances;
	static WSADATA wsaData;
	static bool isInited;
	bool isValid;
	bool isConnected;
	SOCKET  conn_socket;

	char ClientName[PFGW_NETWORK_V2_MACNAME_LEN];
	char ServerID[PFGW_NETWORK_V2_MACNAME_LEN];
	bool bTalkedToServer;
	int  nServerVersion;
	bool m_bCompressionOK;
	bool m_bCompositesOk;

public:
	int Error;
	int Connect(char *host,unsigned short remoteport);
	void Close();
	int Send(char *str, bool bFORCE_NO_Compress=false);
	tcp_ver2_xfer *Recv();

	tcpip_client2(const char *ClientName=0);
	~tcpip_client2();

	bool bCompressionOK() {return m_bCompressionOK;}
	bool bCompositesOk() {return m_bCompositesOk;}
};

#endif
