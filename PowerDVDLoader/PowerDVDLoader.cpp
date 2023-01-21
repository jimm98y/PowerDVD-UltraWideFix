// PowerDVDFullscreen.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "PowerDVDLoader.h"

#define MAX_LOADSTRING 100
#define MAX_STRING 260
#define POLLING_TIMER 1

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void ResizeMovieWindow(HWND hWnd, RECT* parentRect);
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
void MakeFullscreen();
void RegisterShortcut(HWND hWnd);
void ForceRedraw();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_POWERDVDFULLSCREEN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_POWERDVDFULLSCREEN));

	// first try to launch PowerDVD, then wait until it exits. If it does not succeed, just launch the app.
	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;
	if (CreateProcessW(L"PowerDVD.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
	{
		BOOL fQuit = FALSE; // Should the loop terminate?

		while (!fQuit) 
		{
			// Wake when the kernel object is signaled OR
			//  if we have to process a UI message.
			DWORD dwResult =
				MsgWaitForMultipleObjectsEx(
					1, 
					&processInfo.hProcess,
					INFINITE, 
					QS_ALLEVENTS, 
					MWMO_INPUTAVAILABLE);

			switch (dwResult)
			{
			case WAIT_OBJECT_0: // The event became signaled.
				fQuit = TRUE;
				break;

			case WAIT_OBJECT_0 + 1: // A message is in our queue.

				// Dispatch all of the messages.
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT) 
					{
						// A WM_QUIT message, exit the loop
						fQuit = TRUE;
					}
					else
					{
						// Translate and dispatch the message.
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				} // Our queue is empty.
				break;
			}
		}

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
	else
	{
		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_POWERDVDFULLSCREEN));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_POWERDVDFULLSCREEN);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   RegisterShortcut(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL isTimerRunning = FALSE;

	switch (message)
	{
	case WM_HOTKEY:
		if (!isTimerRunning)
		{
			SetTimer(hWnd, POLLING_TIMER, 25, NULL);
			isTimerRunning = TRUE;
		}
		else
		{
			KillTimer(hWnd, POLLING_TIMER);
			isTimerRunning = FALSE;
			ForceRedraw();
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd, POLLING_TIMER);
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		MakeFullscreen();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam) {
	wchar_t caption[MAX_STRING];

	GetWindowTextW(hWnd, caption, MAX_STRING);

	if (wcscmp(caption, L"ActiveMovie Window") == 0)
	{
		// resize the window in order to remove black bars
		ResizeMovieWindow(hWnd, (RECT*)lParam);
	}

	return TRUE;
}

void ForceRedraw()
{
	HWND hWnd = NULL;

	// find the PowerDVD window
	hWnd = FindWindow(NULL, L"PowerDVD");

	if (hWnd != NULL)
	{
		RECT windowRect;
		GetWindowRect(hWnd, &windowRect);

		// this will force-redraw the window in order to return it to its previous dimensions
		SetWindowPos(hWnd, NULL, windowRect.left, windowRect.top, windowRect.right - windowRect.left + 1, windowRect.bottom - windowRect.top + 1, NULL);
		SetWindowPos(hWnd, NULL, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL);
	}
}

void MakeFullscreen()
{
	HWND hWnd = NULL;
	
	// find the PowerDVD window
	hWnd = FindWindow(NULL, L"PowerDVD");

	if (hWnd != NULL)
	{
		WINDOWPLACEMENT placement;
		placement.length = sizeof(placement);

		// get the window's position and size
		GetWindowPlacement(hWnd, &placement);

		// enumerate all child windows of the PowerDVD window
		EnumChildWindows(hWnd, EnumChildProc, (LPARAM)&placement.rcNormalPosition);
	}
}

void ResizeMovieWindow(HWND hWnd, RECT* parentRect)
{
	WINDOWPLACEMENT placement;
	placement.length = sizeof(placement);

	// get window's position and size
	if (GetWindowPlacement(hWnd, &placement))
	{
		placement.showCmd = SW_SHOW;

		const double ratioWithoutBorders = 0.41841;
		const double ratioWithBorders = 0.56179775;

		LONG width = parentRect->right - parentRect->left;
		LONG heightDifference = (LONG)((width * ratioWithBorders - width * ratioWithoutBorders) / 2);

		if (placement.rcNormalPosition.left != 0 ||
			placement.rcNormalPosition.top != -heightDifference ||
			placement.rcNormalPosition.right != width ||
			placement.rcNormalPosition.bottom != (LONG)(width * ratioWithoutBorders - heightDifference))
		{
			placement.rcNormalPosition.left = 0;
			placement.rcNormalPosition.top = -heightDifference;
			placement.rcNormalPosition.right = width;
			placement.rcNormalPosition.bottom = (LONG)(width * ratioWithBorders - heightDifference);
		}

		SetWindowPlacement(hWnd, &placement);
	}
}

void RegisterShortcut(HWND hWnd)
{
	// ctrl + alt + f will toggle fullscreen
	RegisterHotKey(hWnd, 100, MOD_ALT | MOD_CONTROL, 'F');
}
