#pragma once

struct PopupData {
	HWND parent;
	HINSTANCE instance;
	LPWSTR sname;
};

DWORD WINAPI PopupThread(LPVOID pData);