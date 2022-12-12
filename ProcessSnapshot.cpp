#include "ProcessSnapshot.h"
#include <TlHelp32.h>
#include <stdexcept>

using namespace std;

void ProcessSnapshot::takeSnapshot()
{
	nameMap.clear();

	constexpr int N = 2048;

	PROCESSENTRY32 pe32;
	memset(&pe32, 0, sizeof(pe32));
	pe32.dwSize = sizeof(PROCESSENTRY32);

	char buf[N];

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		throw runtime_error("Invalid handle");
	}

	BOOL hasNext = Process32First(hSnap, &pe32);
	while (hasNext) {
		WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, buf, N, NULL, NULL);
		string s = buf;
		nameMap.insert(make_pair(s, pe32.th32ProcessID));

		hasNext = Process32Next(hSnap, &pe32);
	}
}

vector<DWORD> ProcessSnapshot::getPids(const string& name) const
{
	vector<DWORD> pids;
	for (const auto &p : nameMap) {
		if (p.first == name) {
			pids.push_back(p.second);
		}
	}
	return pids;
}
