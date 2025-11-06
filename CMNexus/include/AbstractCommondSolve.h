#pragma once
#include<iostream>
#include<vector>
#include<map>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

class AbstractCommondSolve {
public:
	AbstractCommondSolve() = default;
	~AbstractCommondSolve() = default;
	virtual std::pair<std::string, int> GetCommondGroup(std::string commonds) = 0;
	virtual void AddCommondsFirst(std::string commondsFirst);
protected:
	std::vector<std::string> commondsFirst;
};