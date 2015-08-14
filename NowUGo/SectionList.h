

class SectionList
{
public:
	SectionList();
	~SectionList();
	const LPCWSTR fName = L"sections.txt";
	int err();
	LPCWSTR errStr();
	int getNNames();
	LPCWSTR getName(int index);
private:
	int errnumber;
	static const int MAXSECTIONS = 128;
	LPWSTR sNames[MAXSECTIONS];
	int nNames;
};

