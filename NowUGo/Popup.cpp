#include "stdafx.h"
#include <time.h>
#include "Popup.h"
#include "ClassList.h"
#include "Resource.h"
#include "gfx.h"

static bool		isPopupWndRegistered = false;
static float	nameListPos = 0.0f;
ClassList *		pClassList = NULL;
RecordSet		kidList;
UINT_PTR		timerID = NULL;
BOOL			animationDone = false;
BOOL			showCreditBox = false;

LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_TIMER:
		if (animationDone) break;
		nameListPos += GFX::TextSpeed;
		if (nameListPos > (kidList.nRecords-1)) {
			nameListPos = (float) (kidList.nRecords - 1);
			KillTimer(hWnd, timerID);
			timerID = NULL;
			animationDone = true;
			showCreditBox = true;
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_CREATE:
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// Create an off-screen DC for double-buffering
		HDC hdcM = CreateCompatibleDC(hdc);
		RECT cR;  GetClientRect(hWnd, &cR);
		HBITMAP hbmMem = CreateCompatibleBitmap(hdc, cR.right, cR.bottom);
		HGDIOBJ hOld = SelectObject(hdcM, hbmMem);

		HBRUSH oldB = (HBRUSH) SelectObject(hdcM, CreateSolidBrush(GFX::backgr));
		HPEN oldP = (HPEN) SelectObject(hdcM, CreatePen(PS_SOLID, 1, GFX::backgr));
		
		Rectangle(hdcM, 0, 0, cR.right+1, cR.bottom+1);
		SetMapMode(hdcM, MM_TEXT);
		HFONT font = CreateFont((UINT)(cR.bottom * GFX::TextHeight), 0, 0, 0, GFX::TextWeight,
			GFX::isItalic, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, DEFAULT_PITCH | GFX::TextFamily, NULL);
		HFONT oldF = (HFONT)SelectObject(hdcM, font);
		SetTextColor(hdcM, GFX::textcolor);
		SetBkColor(hdcM, GFX::backgr);
		// draw two names, the floor of nameListPos and the ceil of nameListPos
		UINT n1 = (int)nameListPos;  UINT n2 = n1 + 1;
		float fp = nameListPos - n1;
		if ( (n1 >= 0) && (n1 < kidList.nRecords)) {
			RECT cR1 = cR;
			float off = fp * cR.bottom;
			cR1.top -= (int) off; cR1.bottom -= (int) off;
			DrawText(hdcM, kidList.pRecords[n1]->name, wcslen(kidList.pRecords[n1]->name)
				, &cR1, DT_CENTER | DT_VCENTER);
		}
		if ((n2 >= 0) && (n2 < kidList.nRecords)) {
			RECT cR2 = cR;
			float off = ((fp-1.0f) * cR.bottom);
			cR2.top -= (int) off; cR2.bottom -= (int) off;
			DrawText(hdcM, kidList.pRecords[n2]->name, wcslen(kidList.pRecords[n2]->name)
				, &cR2, DT_CENTER | DT_VCENTER);
		}


		DeleteObject(SelectObject(hdcM, oldB));
		DeleteObject(SelectObject(hdcM, oldP));
		DeleteObject(SelectObject(hdcM, oldF));

		// Transfer the off-screen DC to the screen
		BitBlt(hdc, 0, 0, cR.right, cR.bottom, hdcM, 0, 0, SRCCOPY);

		// Free-up the off-screen DC
		SelectObject(hdcM, hOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcM);

		EndPaint(hWnd, &ps);
		if (showCreditBox) {
			showCreditBox = false;
			UINT msg = MessageBox(hWnd, L"Did student answer?", L"Credit"
				, MB_YESNOCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
			UINT n = kidList.nRecords - 1;
			switch (msg) {
			case IDYES:
				kidList.pRecords[n]->credits++;
			case IDNO:
				kidList.pRecords[n]->turns++;
				break;
			}
			PostQuitMessage(0);
		}
	}
	break;
	case WM_ERASEBKGND:
		return true;
		break;
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
	animationDone = false;
	showCreditBox = false;
	kidList.pRecords = NULL;
	kidList.nRecords = 0;
	pClassList = new ClassList(pD->sname);
	srand((unsigned)time(NULL));

	if (pClassList->err()) {
		MessageBox(NULL, pClassList->errStr(), L"Error", MB_OK);
		goto cleanup;
	}

	kidList = pClassList->getRecordSet();
	nameListPos = 0.0f;

	if (!isPopupWndRegistered) {
		if (!PopupRegisterClass(pD->instance)) goto cleanup;
		isPopupWndRegistered = true;
	}

	UINT sX = GetSystemMetrics(SM_CXSCREEN);
	UINT sY = GetSystemMetrics(SM_CYSCREEN);

	HWND hWnd = CreateWindowExW(WS_EX_LEFT, L"NowUGo Banner", L"Banner!", 
		WS_POPUP | WS_VISIBLE | WS_SYSMENU,
		0, (UINT) (sY * GFX::YPos), sX, (UINT) (sY * GFX::YSize)
		, nullptr , nullptr,
		pD->instance, nullptr);

	if (!hWnd) goto cleanup;


	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
	SetForegroundWindow(hWnd);
	// Start animation timer
	timerID = SetTimer(hWnd, timerID, 50, NULL);

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
	if (kidList.pRecords) delete[] kidList.pRecords;
	kidList.nRecords = 0;
	SendMessage(pHwnd, WM_USER_HKLISTEN, 0, 0);
	return 0;
}
