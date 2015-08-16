#pragma once

// if your name is longer than 256 characters we'll just have to abbreviate it
struct StudentRecord
{
	static const	UINT	MAXNAME = 256;
					WCHAR	name[MAXNAME];
					UINT	turns;
					UINT	credits;
};

struct RecordSet
{
	StudentRecord **	pRecords;
	UINT				nRecords;
};

// if my class has more than 512 students I'm complaining to the principal
class ClassList
{
public:
	ClassList(LPCWSTR sname);
	~ClassList();
	int err();
	LPCWSTR errStr();
	static const UINT MAXRECORDS = 512;
	StudentRecord Records[MAXRECORDS];
	UINT nRecords;
	RecordSet getRecordSet();
private:
	int errnumber;
	WCHAR fname[256];
};
