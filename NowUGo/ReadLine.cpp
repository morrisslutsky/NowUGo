#include "stdafx.h"
#include "ReadLine.h"

BOOL ReadLine(HANDLE hFile, char *pszBuffer, DWORD dwSize)
{
	DWORD i, dwRead;

	if (pszBuffer == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (!ReadFile(hFile, pszBuffer, dwSize, &dwRead, NULL) || (dwRead == 0))
	{
		SetLastError(ERROR_HANDLE_EOF);
		return FALSE;
	}

	for (i = 0; i < dwRead; i++)
	{
		BYTE c = ((BYTE *)pszBuffer)[i];
		if (c == '\r')
		{
			((BYTE *)pszBuffer)[i] = 0;
			if (i + 1 < dwRead && ((BYTE *)pszBuffer)[i + 1] == '\n')
			{
				i++;
			}
			break;
		}
		else if (c == '\n')
		{
			((BYTE *)pszBuffer)[i] = 0;
			break;
		}
	}

	if (i >= dwRead)
	{
		((BYTE *)pszBuffer)[i] = 0;
	}
	else
	{
		i++;
	}

	SetFilePointer(hFile, i - dwRead, NULL, FILE_CURRENT);

	return TRUE;
}