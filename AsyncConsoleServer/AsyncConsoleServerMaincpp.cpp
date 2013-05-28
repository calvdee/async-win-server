#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

int main()
{
	WSADATA WsaDat;
	int nResult=WSAStartup(MAKEWORD(2,2),&WsaDat);
	if(nResult!=0)
	{
		std::cout<<"WSA Initialization failed: "<<nResult<<"\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	SOCKET Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(Socket==INVALID_SOCKET)
	{
		int nError=WSAGetLastError();
		std::cout<<"Socket creation failed: "<<nError<<"\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	SOCKADDR_IN serverInf;
	serverInf.sin_family=AF_INET;
	serverInf.sin_addr.s_addr=INADDR_ANY;
	serverInf.sin_port=htons(8888);

	if(bind(Socket,(SOCKADDR*)(&serverInf),sizeof(serverInf))==SOCKET_ERROR)
	{
		int nError=WSAGetLastError();
		std::cout<<"Unable to bind socket: "<<nError<<"\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	if(listen(Socket,1)==SOCKET_ERROR)
	{
		int nError=WSAGetLastError();
		std::cout<<"Unable to listen: "<<nError<<"\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	SOCKET TempSock=SOCKET_ERROR;
	while(TempSock==SOCKET_ERROR)
	{
		std::cout<<"Waiting for incoming connections...\r\n";
		TempSock=accept(Socket,NULL,NULL);
	}
	Socket=TempSock;
	std::cout<<"Client connected!\r\n\r\n";

	u_long iMode=1;
	ioctlsocket(Socket,FIONBIO,&iMode);
	
	for(;;)
	{
		char *szMessage="Welcome to the server!\r\n";
		send(Socket,szMessage,strlen(szMessage),0);
						
		int nError=WSAGetLastError();
		if(nError!=WSAEWOULDBLOCK&&nError!=0)

		{
			std::cout<<"Winsock error code: "<<nError<<"\r\n";
			std::cout<<"Client disconnected!\r\n";

			// Shutdown our socket
			shutdown(Socket,SD_SEND);

			// Close our socket entirely
			closesocket(Socket);

			break;
		}
	
		Sleep(1000);
	}

	// Shutdown our socket
	if(shutdown(Socket,SD_SEND)==SOCKET_ERROR)
	{
		int nError=WSAGetLastError();
		std::cout<<"Unable to shutdown socket: "<<nError<<"\r\n";
		closesocket(Socket);
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	// Close our socket

	closesocket(Socket);
	while(WSAGetLastError()==WSAEINPROGRESS) {
		std::cout<<"Winsock Error: WSAEINPROGRESS, retrying...\r\n";
		Sleep(1000);
		closesocket(Socket);
	}

	// Cleanup Winsock
	WSACleanup();
	system("PAUSE");
	return 0;
}