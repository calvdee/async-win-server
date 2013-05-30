#include <winsock2.h>
#include <windows.h>
#include <string>
#include "global.h"

#pragma comment(lib,"ws2_32.lib")

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
			SetDlgItemText(hWnd, IDC_EDIT_IN, L"Server responses...");

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

			// Initialize Winsock
			InitWinsock(hWnd,szServer,nPort,Socket);
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

					//SendMessage(hEditIn,
					//	WM_SETTEXT,
					//	sizeof(szIncoming)-1,
					//	reinterpret_cast<LPARAM>(&szHistory));
					std::string sHistory = std::string(szHistory);
					SetDlgItemText(hWnd, IDC_EDIT_IN, std::wstring(sHistory.begin(),sHistory.end()).c_str());
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