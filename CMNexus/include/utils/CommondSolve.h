   #pragma once
#include"../include/Abstract/AbstractCommondSolve.h"
#include<functional>
#include<regex>

struct DataHeader;
struct AutoAdapt;

class CMCommondSolve : public AbstractCommondSolve {
	/*
	命令.目标 -> 例如: SendMessage.256(已弃用)

	/命令 目标 内容 -> 例如: /SendFile user 你好
	*/
public:
	void AddCommondsFirst(std::string first, std::function<void(DataHeader*, int)> func);
	std::pair<std::string, int> GetCommondGroup(std::string commonds);
	bool GetCommondGroup(std::string commond, std::vector<std::string>& parts);
	std::vector<AutoAdapt>& GetCommondsVector();
private:
	std::vector<AutoAdapt> commonds;
};


struct AutoAdapt {
	std::string commond;
	std::function<void(DataHeader*, int)> solveFunction;
};