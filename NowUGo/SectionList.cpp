#include "stdafx.h"
#include "SectionList.h"
#include "ReadLine.h"

SectionList::SectionList()
{
	errnumber = 0;
	nNames = 0;
	HANDLE hFile;
	hFile = CreateFile(fName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		errnumber = 1;
		return;
	}
	
	const int MAXSZ = 1024;
	char lineBuffer[MAXSZ];
	while (ReadLine(hFile, lineBuffer, MAXSZ)) {
		int ll = strlen(lineBuffer); int i;
		wchar_t * wideLine = new wchar_t[ll + 1];
		for (i = 0; i < ll; i++) { wideLine[i] = (wchar_t)lineBuffer[i]; }
		wideLine[i] = 0;
		sNames[nNames] = wideLine;
		nNames++;
		if (nNames == MAXSECTIONS) break;
	}
	if (nNames == 0) {
		errnumber = 2;
	}
	CloseHandle(hFile);
}

int SectionList::getNNames() { return nNames; }
LPCWSTR SectionList::getName(int index) { return sNames[index]; }

int SectionList::err() {
	return errnumber;
}

LPCWSTR SectionList::errStr() {
	switch (errnumber) {
	case 1:
		return L"Cannot open sections.txt";
		break;
	case 2:
		return L"Error reading sections.txt";
		break;
	}
	return L"No error";
}


SectionList::~SectionList()
{
	for (int i = 0; i < nNames; i++) {
		delete sNames[i];
	}
}
