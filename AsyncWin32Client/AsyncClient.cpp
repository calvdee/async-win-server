#include <winsock2.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

#define IDC_EDIT_IN		101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define WM_SOCKET		104

char *szServer="localhost";
int nPort=5555;

HWND hEditIn=NULL;
HWND hEditOut=NULL;
SOCKET Socket=NULL;
char szHistory[10000];

LRESULT CALLBACK WinProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd)
{
	WNDCLASSEX wClass;
	ZeroMemory(&wClass,sizeof(WNDCLASSEX));
	wClass.cbClsExtra=NULL;
	wClass.cbSize=sizeof(WNDCLASSEX);
	wClass.cbWndExtra=NULL;
	wClass.hbrBackground=(HBRUSH)COLOR_WINDOW;
	wClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wClass.hIcon=NULL;
	wClass.hIconSm=NULL;
	wClass.hInstance=hInst;
	wClass.lpfnWndProc=(WNDPROC)WinProc;
	wClass.lpszClassName=L"Window Class";
	wClass.lpszMenuName=NULL;
	wClass.style=CS_HREDRAW|CS_VREDRAW;

	if(!RegisterClassEx(&wClass))
	{
		int nResult=GetLastError();
		MessageBox(NULL,
			L"Window class creation failed\r\nError code:",
			L"Window Class Failed",
			MB_ICONERROR);
	}

	HWND hWnd=CreateWindowEx(NULL,
			L"Window Class",
			L"Windows Async Client",
			WS_OVERLAPPEDWINDOW,
			200,
			200,
			640,
			480,
			NULL,
			NULL,
			hInst,
			NULL);

	if(!hWnd)
	{
		int nResult=GetLastError();

		MessageBox(NULL,
			L"Window creation failed\r\nError code:",
			L"Window Creation Failed",
			MB_ICONERROR);
	}

    ShowWindow(hWnd,nShowCmd);

	MSG msg;
	ZeroMemory(&msg,sizeof(MSG));

	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			ZeroMemory(szHistory,sizeof(szHistory));

			// Create incoming message box
			hEditIn=CreateWindowEx(WS_EX_CLIENTEDGE,
				L"EDIT",
				L"",
				WS_CHILD|WS_VISIBLE|ES_MULTILINE|
				ES_AUTOVSCROLL|ES_AUTOHSCROLL,
				50,
				120,
				400,
				200,
				hWnd,
				(HMENU)IDC_EDIT_IN,
				GetModuleHandle(NULL),
				NULL);
			if(!hEditIn)
			{
				MessageBox(hWnd,
					L"Could not create incoming edit box.",
					L"Error",
					MB_OK|MB_ICONERROR);
			}
			HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEditIn,
				WM_SETFONT,
				(WPARAM)hfDefault,
				MAKELPARAM(FALSE,0));
			SendMessage(hEditIn,
				WM_SETTEXT,
				NULL,
				(LPARAM)"Attempting to connect to server...");

			// Create outgoing message box
			hEditOut=CreateWindowEx(WS_EX_CLIENTEDGE,
						L"EDIT",
						L"",
						WS_CHILD|WS_VISIBLE|ES_MULTILINE|
						ES_AUTOVSCROLL|ES_AUTOHSCROLL,
						50,
						50,
						400,
						60,
						hWnd,
						(HMENU)IDC_EDIT_IN,
						GetModuleHandle(NULL),
						NULL);
			if(!hEditOut)
			{
				MessageBox(hWnd,
					L"Could not create outgoing edit box.",
					L"Error",
					MB_OK|MB_ICONERROR);
			}

			SendMessage(hEditOut,
				WM_SETFONT,(WPARAM)hfDefault,
				MAKELPARAM(FALSE,0));
			SendMessage(hEditOut,
				WM_SETTEXT,
				NULL,
				(LPARAM)"Type message here...");

			// Create a push button
			HWND hWndButton=CreateWindow( 
					    L"BUTTON",
						L"Send",
						WS_TABSTOP|WS_VISIBLE|
						WS_CHILD|BS_DEFPUSHBUTTON,
						50,	
						330,
						75,
						23,
						hWnd,
						(HMENU)IDC_MAIN_BUTTON,
						GetModuleHandle(NULL),
						NULL);
			
			SendMessage(hWndButton,
				WM_SETFONT,
				(WPARAM)hfDefault,
				MAKELPARAM(FALSE,0));

			// Set up Winsock
			WSADATA WsaDat;
			int nResult=WSAStartup(MAKEWORD(2,2),&WsaDat);
			if(nResult!=0)
			{
				MessageBox(hWnd,
					L"Winsock initialization failed",
					L"Critical Error",
					MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			if(Socket==INVALID_SOCKET)
			{
				MessageBox(hWnd,
					L"Socket creation failed",
					L"Critical Error",
					MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			nResult=WSAAsyncSelect(Socket,hWnd,WM_SOCKET,(FD_CLOSE|FD_READ));
			if(nResult)
			{
				MessageBox(hWnd,
					L"WSAAsyncSelect failed",
					L"Critical Error",
					MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
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
				break;
			}

			// Set up our socket address structure
			SOCKADDR_IN SockAddr;
			SockAddr.sin_port=htons(nPort);
			SockAddr.sin_family=AF_INET;
			SockAddr.sin_addr.s_addr=*((unsigned long*)host->h_addr);

			connect(Socket,(LPSOCKADDR)(&SockAddr),sizeof(SockAddr));
		}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
            {
				case IDC_MAIN_BUTTON:
				{
					char szBuffer[1024];

					int test=sizeof(szBuffer);
					ZeroMemory(szBuffer,sizeof(szBuffer));

					SendMessage(hEditOut,
						WM_GETTEXT,
						sizeof(szBuffer),
						reinterpret_cast<LPARAM>(szBuffer));
					send(Socket,szBuffer,strlen(szBuffer),0);
					SendMessage(hEditOut,WM_SETTEXT,NULL,(LPARAM)"");
				}
				break;
			}
			break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			shutdown(Socket,SD_BOTH);
			closesocket(Socket);
			WSACleanup();
			return 0;
		}
		break;

		case WM_SOCKET:
		{
			if(WSAGETSELECTERROR(lParam))
			{	
				MessageBox(hWnd,
					L"Connection to server failed",
					L"Error",
					MB_OK|MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}
			switch(WSAGETSELECTEVENT(lParam))
			{
				case FD_READ:
				{
					char szIncoming[1024];
					ZeroMemory(szIncoming,sizeof(szIncoming));

					int inDataLength=recv(Socket,
						(char*)szIncoming,
						sizeof(szIncoming)/sizeof(szIncoming[0]),
						0);

					strncat_s(szHistory,szIncoming,inDataLength);
					strcat_s(szHistory,"\r\n");

					SendMessage(hEditIn,
						WM_SETTEXT,
						sizeof(szIncoming)-1,
						reinterpret_cast<LPARAM>(&szHistory));
				}
				break;

				case FD_CLOSE:
				{
					MessageBox(hWnd,
						L"Server closed connection",
						L"Connection closed!",
						MB_ICONINFORMATION|MB_OK);
					closesocket(Socket);
					SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				}
				break;
			}
		} 
	}

	return DefWindowProc(hWnd,msg,wParam,lParam);
}