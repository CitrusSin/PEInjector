#pragma once
#include <Windows.h>
#include <vector>
#include <unordered_map>
#include <string>

class ProcessSnapshot
{
public:
	void takeSnapshot();
	std::vector<DWORD> getPids(const std::string& name) const;
private:
	std::unordered_multimap<std::string, DWORD> nameMap;
};

