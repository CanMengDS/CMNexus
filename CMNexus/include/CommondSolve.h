   #pragma once
#include"AbstractCommondSolve.h"
#include<regex>

class CMCommondSolve : public AbstractCommondSolve {
	/*
	命令一般由:

	命令.目标 -> 例如: SendMessage.256

	这样的示例组成
	*/
public:
	std::pair<std::string, int> GetCommondGroup(std::string commonds);
};
