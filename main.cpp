#include <iostream>
#include <string>
#include <stdexcept>
#include <functional>
#include <Windows.h>

#include "Argument.h"
#include "ProcessSnapshot.h"

using namespace std;

bool promoteDebugPrivilege() {
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	if (!OpenProcessToken(
		GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&hToken
		)) {
		return false;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		CloseHandle(hToken);
		return false;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
		CloseHandle(hToken);
		return false;
	}
	CloseHandle(hToken);
	return true;
}

void injectDll(DWORD pid, const string& dllName) {
	HANDLE hPc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	HMODULE hKrnl32 = LoadLibraryA("kernel32");
	if (hKrnl32 == NULL) {
		throw runtime_error("Failed to load kernel32.dll");
	}
	void* proc = GetProcAddress(hKrnl32, "LoadLibraryA");
	
	char fullName[2048];
	GetFullPathNameA(dllName.c_str(), 2048, fullName, NULL);

	DWORD memsize = strlen(fullName) + 1;
	LPVOID ptr = VirtualAllocEx(hPc, NULL, memsize, MEM_COMMIT, PAGE_READWRITE);
	if (ptr == NULL) {
		CloseHandle(hPc);
		throw runtime_error("Failed to alloc memory space into target");
	}
	WriteProcessMemory(hPc, ptr, fullName, memsize, NULL);
	HANDLE hThread = CreateRemoteThread(hPc, NULL, 0, (LPTHREAD_START_ROUTINE)proc, ptr, NULL, NULL);
	if (hThread == NULL) {
		VirtualFreeEx(hPc, ptr, 0, MEM_RELEASE);
		CloseHandle(hPc);
		throw runtime_error("Failed to create remote thread");
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	VirtualFreeEx(hPc, ptr, 0, MEM_RELEASE);
	CloseHandle(hPc);
}

vector<DWORD> castVector(const vector<string>& src) {
	vector<DWORD> result;
	char* eptr;
	for (const string& el : src) {
		result.push_back(strtol(el.c_str(), &eptr, 10));
	}
	return result;
}

template <typename A, typename B, typename Converter>
vector<A> castVector(const vector<B>& src, Converter& conv) {
	vector<A> result;
	for (const B& srcElement : src) {
		result.push_back(conv(srcElement));
	}
	return result;
}

void showHelpMessage() {
	cout << "PEInjector Usage:" << endl;
	cout << "PEInjector [-p <Dll File Name>] [-im <Process Name>] [-pid <Process ID>]" << endl;
}

int main(int argc, char **argv) {
	if (!promoteDebugPrivilege())
	{
		cerr << "Failed to get debug privilege." << endl;
		return 0;
	}

	Argument arg(argc-1, argv+1);

	vector<string> peNames = arg.getParameters("p");
	vector<DWORD> targetPids = castVector(arg.getParameters("pid"));
	vector<string> targetProcessNames = arg.getParameters("im");

	// Add PIDs from process names
	ProcessSnapshot ps;
	ps.takeSnapshot();
	for (string& pName : targetProcessNames)
	{
		for (DWORD pid : ps.getPids(pName))
		{
			targetPids.push_back(pid);
		}
	}

	if (peNames.empty() || targetPids.empty()) 
	{
		if (!targetProcessNames.empty()) 
		{
			cout << "Process Not Found!" << endl;
		}
		else 
		{
			showHelpMessage();
		}
		return 0;
	}

	cout << "DLLs to inject: " << endl;
	for (string& name : peNames)
	{
		cout << name << endl;
	}
	cout << endl;
	cout << "Processes(PID) to be injected: " << endl;
	int cnt = 0;
	for (DWORD pid : targetPids)
	{
		cout << pid << " ";
		if (++cnt % 10 == 0)
		{
			cout << endl;
		}
	}
	cout << endl << endl;

	for (DWORD pid : targetPids)
	{
		for (const string& dllName : peNames)
		{
			cout << "Injecting " << dllName << " to PID " << pid << endl;
			try
			{
				injectDll(pid, dllName);
			}
			catch (runtime_error e)
			{
				cerr << "Failed to inject " << dllName << " to PID " << pid << ": " << endl;
				cerr << e.what() << endl;
			}
			cout << "Injected." << endl;
		}
	}
	return 0;
}