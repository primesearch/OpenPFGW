#if !defined (__TCPIP_Client_H__)
#define __TCPIP_Client_H__

#include <winsock.h>

class tcpip_client {
	static struct hostent * GetHost(char *host);
	static int instances;
	static WSADATA wsaData;
	static bool isInited;
	bool isValid;
	bool isConnected;
	SOCKET  conn_socket;
public:
	int Error;
	char Buffer[16384];
	int Connect(char *host,unsigned short remoteport);
	void Close();
	int Send(char *str);
	char *Recv(int time=0);

	tcpip_client();
	~tcpip_client();
};

#endif
