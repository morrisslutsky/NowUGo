#include "stdafx.h"
#include "ReadLine.h"
#include "ClassList.h"

ClassList::ClassList(LPCWSTR sname) {
	errnumber = 0;
	nRecords = 0;
	wsprintf(fname, L"%s.txt", sname);
	HANDLE hFile;
	hFile = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		errnumber = 1;
		goto fail;
	}

	char lineBuffer[StudentRecord::MAXNAME];
	WCHAR wideLine[StudentRecord::MAXNAME];
	while (ReadLine(hFile, lineBuffer, StudentRecord::MAXNAME)) {
		int ll = strlen(lineBuffer); int i;
		for (i = 0; i < ll; i++) { wideLine[i] = (wchar_t)lineBuffer[i]; }
		wideLine[i] = 0;
		int firstComma = 0;
		int secondComma = 0;
		for (i = 0; i < ll; i++) { 
			if (wideLine[i] == ',') {
				firstComma = i++;
				break;
			}
		}
		for (; i < ll; i++) {
			if (wideLine[i] == ',') {
				secondComma = i++;
				break;
			}
		}
		// cut it up and store it
		if (firstComma) wideLine[firstComma] = 0;
		if (secondComma) wideLine[secondComma] = 0;
		lstrcpyW(Records[nRecords].name, wideLine);
		Records[nRecords].turns = firstComma?_wtoi(&wideLine[firstComma + 1]):0;
		Records[nRecords].credits = secondComma ? _wtoi(&wideLine[secondComma + 1]) : 0;
		nRecords++;
	}
	CloseHandle(hFile);
fail:
	return;
}

int ClassList::err() { return errnumber; }

LPCWSTR ClassList::errStr() {
	switch (errnumber) {
	case 0:
		return L"No error";
	case 1:
		return L"Error opening class roster";
	case 2:
		return L"Error writing class roster";
	}
	return L"Other error?";
}

ClassList::~ClassList() { 
	// Did we successfully load up the roster?  Otherwise bail.
	if (errnumber) return;
	// Back up existing file for safety
	WCHAR bfile[256];
	lstrcpy(bfile, fname);
	bfile[lstrlen(bfile) - 1] = '0';
	CopyFile(fname, bfile, false);
	// Write out class list in CSV format
	HANDLE outFile = CreateFile(fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (outFile == INVALID_HANDLE_VALUE) {
		errnumber = 2;
		return;
	}
	char lineBuffer[StudentRecord::MAXNAME * 2];
	WCHAR wBuf[StudentRecord::MAXNAME * 2];
	UINT i, j;
	for (i = 0; i < nRecords; i++) {
		wsprintf(wBuf, L"%s,%d,%d\r\n", Records[i].name, Records[i].turns, Records[i].credits);
		j = 0;  
		while (wBuf[j]) {
			lineBuffer[j] = (char)wBuf[j];
			j++;
		}
		lineBuffer[j] = 0;
		WriteFile(outFile, lineBuffer, strlen(lineBuffer), NULL, NULL);
	}
	CloseHandle(outFile);
}