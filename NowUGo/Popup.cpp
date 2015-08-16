#include "stdafx.h"
#include "Popup.h"
#include "ClassList.h"
#include "Resource.h"

static bool isPopupWndRegistered = false;
ClassList * pClassList = NULL;

LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	//case WM_ERASEBKGND:
	//	return true;
	//	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM PopupRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = PopupWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOWUGO));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = NULL; 
	wcex.lpszClassName = L"NowUGo Banner";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}



DWORD WINAPI PopupThread(LPVOID pData) {
	PopupData * pD = (PopupData *)pData;

	pClassList = new ClassList(pD->sname);

	if (pClassList->err()) {
		MessageBox(NULL, pClassList->errStr(), L"Error", MB_OK);
		goto cleanup;
	}
	if (!isPopupWndRegistered) {
		if (!PopupRegisterClass(pD->instance)) goto cleanup;
		isPopupWndRegistered = true;
	}

	HWND hWnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, L"NowUGo Banner", L"Banner!", 
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		0, 0, 300, 300
		, nullptr , nullptr,
		pD->instance, nullptr);

	if (!hWnd) goto cleanup;


	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
	SetForegroundWindow(hWnd);
	// Popup thread message loop:
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	}


cleanup:
	// Thread Cleanup
	HWND pHwnd = pD->parent;
	delete pD->sname;
	delete pD;
	delete pClassList;
	SendMessage(pHwnd, WM_USER_HKLISTEN, 0, 0);
	return 0;
}
