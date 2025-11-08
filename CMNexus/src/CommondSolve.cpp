#include "../include/CommondSolve.h"
#include "../include/CMNetDefs.h"

void CMCommondSolve::AddCommondsFirst(std::string first, std::function<void(DataHeader*, int)> func)
{
	commonds.push_back({ "/" + first,func });
}

std::pair<std::string, int> CMCommondSolve::GetCommondGroup(std::string commonds)
{
	return std::pair<std::string, int>();
}

bool CMCommondSolve::GetCommondGroup(std::string commond, std::vector<std::string>& parts)
{
	std::regex rex("^/([a-zA-Z]{1,10})(\\s+\\S+)+$");
	if (std::regex_match(commond, rex)) {
		std::regex spaceGap("\\s");
		std::sregex_token_iterator it(commond.begin(), commond.end(), spaceGap, -1);
		std::sregex_token_iterator end;

		while (it != end) {
			parts.push_back(*it);
			it++;
		}
	}
	return true;
}

std::vector<AutoAdapt>& CMCommondSolve::GetCommondsVector()
{
	return commonds;
}
