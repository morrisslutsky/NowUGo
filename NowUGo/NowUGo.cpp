// NowUGo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "NowUGo.h"
#include "SectionList.h"
#include "Popup.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
SectionList * pSlData = NULL;
HWND g_hComboSection;
char g_hotKey = 'Z';
RAWINPUTDEVICE rid;
BOOL g_acceptHotKey = true;
DWORD popupThreadID = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NOWUGO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NOWUGO));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
		if (IsDialogMessage(msg.hwnd, &msg)) continue;

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
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
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOWUGO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= NULL; //MAKEINTRESOURCEW(IDC_NOWUGO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
   hInst = hInstance; // Store instance handle in our global variable

   SIZE scaleMe;
   BOOL isScale = GetTextExtentPoint32(GetDC(NULL), L"W", 1, &scaleMe);
   int S = scaleMe.cy;
   HWND hWnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
      CW_USEDEFAULT, 0, S*20, S*10, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   RECT cR;
   GetClientRect(hWnd, &cR);

   HWND hWndStatic = CreateWindow(WC_STATIC, TEXT(""), SS_CENTER | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, S * 2, S, cR.right - S * 4, S, hWnd, NULL, hInstance, NULL);
   SetWindowText(hWndStatic, L"Choose Section");

   WCHAR hkTxt[64];
   wsprintf(hkTxt, L"Hotkey CTRL-ALT-%c", g_hotKey);
   HWND hWndStatic2 = CreateWindow(WC_STATIC, TEXT(""), SS_CENTER | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, S * 2, cR.bottom - S, cR.right - S * 4, S, hWnd, NULL, hInstance, NULL);
   SetWindowText(hWndStatic2, hkTxt);

   HWND hWndComboBox = CreateWindow(WC_COMBOBOX, TEXT(""),
	   CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP,
	   S*2, S*2, cR.right-S*4, cR.bottom-S*3, hWnd, NULL, hInstance,
	   NULL);

   pSlData = new SectionList();
   if (pSlData->err()) {
	   MessageBox(hWnd, pSlData->errStr(), L"Error", MB_OK);
	   exit(1);
   }

   int n = pSlData->getNNames();
   for (int i = 0; i < n; i++) {
	   SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)pSlData->getName(i));
   }
   
   SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
   g_hComboSection = hWndComboBox;
  

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_CREATE			- Register raw input 
//  WM_INPUT			- Monitor raw input for hotkey
//  WM_USER_HKLISTEN	- Reactivate raw input 
//  WM_DESTROY			- post a quit message and return
//  WM_PAINT			- dummy handler

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message)
    {
	case WM_CREATE:
		// register interest in raw data here
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.usUsagePage = 1;
		rid.usUsage = 6;
		rid.hwndTarget = hWnd;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_USER_HKLISTEN:
		g_acceptHotKey = true;
		break;

	case WM_INPUT: {
			if (!g_acceptHotKey) break;
			UINT dwSize;
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)) == -1) {
				break;
			}
			LPBYTE lpb = new BYTE[dwSize];
			if (lpb == NULL) {
				break;
			}
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
				delete[] lpb;
				break;
			}
			PRAWINPUT raw = (PRAWINPUT)lpb;
			if (raw->data.keyboard.Message == WM_KEYDOWN) {
				if (raw->data.keyboard.VKey == g_hotKey) {
					if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL)) {
						LPWSTR sName = new wchar_t[1024];
						GetWindowText(g_hComboSection, sName, 1023);
						PopupData * pD = new PopupData();
						pD->parent = hWnd;
						pD->instance = hInst;
						pD->sname = sName;
						CreateThread(NULL, // default security attributes
							0,			// default stack size
							PopupThread,// start address (in Popup.cpp)
							pD,			// pass PopupData
							0,
							&popupThreadID
							);

						g_acceptHotKey = false;
					}
				}
			}
			break;
		}

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


