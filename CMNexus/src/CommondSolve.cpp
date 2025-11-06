#include "CommondSolve.h"

std::pair<std::string, int> CMCommondSolve::GetCommondGroup(std::string commonds)
{
    std::regex fen("\\.");
    std::sregex_token_iterator it(commonds.begin(), commonds.end(), fen, -1);
    std::sregex_token_iterator end;

    std::vector<std::string> parts;
    while (it != end) {
        parts.push_back(*it);
        ++it;
    }
    if (parts.size() == 2) return std::pair<std::string, int>(parts[0], atoi(parts[1].c_str()));
}
