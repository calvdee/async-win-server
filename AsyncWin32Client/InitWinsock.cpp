#include <WinSock2.h>
#include <Windows.h>
#include "global.h"

int InitWinsock(HWND hWnd, char* szServer, int nPort, SOCKET& Socket)
{
	WSADATA WsaDat;
	int nResult=WSAStartup(MAKEWORD(2,2),&WsaDat);
	if(nResult!=0)
	{
		MessageBox(hWnd,
			L"Winsock initialization failed",
			L"Critical Error",
			MB_ICONERROR);
		SendMessage(hWnd,WM_DESTROY,NULL,NULL);
		return -1;
	}

	Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(Socket==INVALID_SOCKET)
	{
		MessageBox(hWnd,
			L"Socket creation failed",
			L"Critical Error",
			MB_ICONERROR);
		SendMessage(hWnd,WM_DESTROY,NULL,NULL);
		return -2;
	}

	nResult=WSAAsyncSelect(Socket,hWnd,WM_SOCKET,(FD_CLOSE|FD_READ));
	if(nResult)
	{
		MessageBox(hWnd,
			L"WSAAsyncSelect failed",
			L"Critical Error",
			MB_ICONERROR);
		SendMessage(hWnd,WM_DESTROY,NULL,NULL);
		return -1;
	}

	// Resolve IP address for hostname
	struct hostent *host;
	if((host=gethostbyname(szServer))==NULL)
	{
		MessageBox(hWnd,
			L"Unable to resolve host name",
			L"Critical Error",
			MB_ICONERROR);
		SendMessage(hWnd,WM_DESTROY,NULL,NULL);
		return -1;
	}

	// Set up our socket address structure
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port=htons(nPort);
	SockAddr.sin_family=AF_INET;
	SockAddr.sin_addr.s_addr=*((unsigned long*)host->h_addr);

	connect(Socket,(LPSOCKADDR)(&SockAddr),sizeof(SockAddr));

	return 0;
}

