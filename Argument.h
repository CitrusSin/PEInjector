#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class Argument
{
public:
	Argument(int argc, char** argv);
	void parseArgument(int argc, char** argv);
	bool hasParameter(const std::string& param) const;
	std::vector<std::string> getParameters(const std::string& param) const;

private:
	std::unordered_multimap<std::string, std::string> parameters;
	std::vector<std::string> arguments;
};

