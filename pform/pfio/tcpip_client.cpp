#ifdef USE_NETWORK

#include <stdlib.h>
#include <string.h>

#include "tcpip_client.h"

int tcpip_client::instances = 0;
bool tcpip_client::isInited=false;
WSADATA tcpip_client::wsaData;

tcpip_client::tcpip_client()
{
	if (isInited==false && WSAStartup(0x202,&wsaData) !=0) 
		isValid=false;
	else {
		isValid=true;
		isInited=true;
	}
	instances++;
	isConnected=false;
}

tcpip_client::~tcpip_client()
{
	if (isConnected)
		closesocket(conn_socket);
	
	if (instances==1 && isInited==true) {
		WSACleanup();
		isInited=false;
	}
	instances--;
}

struct hostent * tcpip_client::GetHost(char *host)
{
	struct hostent *hp;
	unsigned int addr;

	if (isalpha(host[0])) {   /* server address is a name */
		hp = gethostbyname(host);
	}
	else  { /* Convert nnn.nnn address to a usable one */
		addr = inet_addr(host);
		hp = gethostbyaddr((char *)&addr,4,AF_INET);
	}
	return hp;
}

void tcpip_client::Close()
{
	if (isConnected)
		closesocket(conn_socket);
	isConnected=false;
	Error=0;
}

int tcpip_client::Connect(char *host,unsigned short remoteport)
{
	struct hostent *hp;
	struct sockaddr_in server;

	if (isConnected==true || remoteport==0)
		return -1;

	hp=GetHost(host);

	if (hp == NULL ) 
		return -1;

	//
	// Copy the resolved information into the sockaddr_in structure
	//
	memset(&server,0,sizeof(server));
	memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(remoteport);
	
	conn_socket = socket(AF_INET,SOCK_STREAM,0); /* Open a socket */
	if (conn_socket <0 ) 
		return -2;

	if (connect(conn_socket,(struct sockaddr*)&server,sizeof(server)) != 0) {
		Error=WSAGetLastError();
		return -2;
	} else
		isConnected=true;

	Error=0;
	return 0;
}

int tcpip_client::Send(char *str)
{
	if (isConnected==false) {
		Error=-2;
		return -2;
	}
	if (send(conn_socket,str,strlen(str),0)==SOCKET_ERROR) {
		Error=WSAGetLastError();
		return -1;
	} else {
		Error=0;
		return 0;
	}
}

char * tcpip_client::Recv(int time)
{
	int retval,count;
	unsigned long ioctl_opt=1;
	if (isConnected==false) {
		Error=-2;
		return NULL;
	}
	if (time==0) {
		retval=recv(conn_socket,Buffer,16384,0);
		if (retval==SOCKET_ERROR) {
			Error=WSAGetLastError();
			return NULL;
		} 
	} else {
		if (ioctlsocket(conn_socket,FIONBIO,&ioctl_opt)==SOCKET_ERROR) {
			Error=WSAGetLastError();
			return NULL;
		}
		retval=recv(conn_socket,Buffer,16384,0);
		if (retval==SOCKET_ERROR) {
			Error=WSAGetLastError();
			count=0;
			while (Error==WSAEWOULDBLOCK && count<time/30) {
				Sleep(30);
				retval=recv(conn_socket,Buffer,16384,0);
				if (retval==SOCKET_ERROR) 
					Error=WSAGetLastError();
				else
					Error=0;
				count++;
			}
			if (Error!=0) 
				return NULL;
		}
		ioctl_opt=0;
		ioctlsocket(conn_socket,FIONBIO,&ioctl_opt);
	}
	Error=0;
	Buffer[retval]=0;
	return Buffer;
}

#endif
