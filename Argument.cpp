#include "Argument.h"
#include <stdexcept>

using namespace std;

Argument::Argument(int argc, char** argv)
{
	parseArgument(argc, argv);
}

void Argument::parseArgument(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
	{
		string s = argv[i];
		if (s.front() == '-' || s.front() == '/')
		{
			string argName = s.substr(max(s.find_first_not_of('-'), s.find_first_not_of('/')));
			if (i + 1 < argc)
			{
				parameters.insert(make_pair(argName, argv[++i]));
			}
			else
			{
				parameters.insert(make_pair(argName, "true"));
			}
		}
		else
		{
			arguments.emplace_back(argv[i]);
		}
	}
}

bool Argument::hasParameter(const std::string& param) const
{
	return parameters.count(param) > 0;
}

vector<std::string> Argument::getParameters(const std::string& param) const
{
	vector<string> pairs;
	for (const pair<string, string>& p : parameters)
	{
		if (p.first == param)
		{
			pairs.push_back(p.second);
		}
	}
	return pairs;
}
