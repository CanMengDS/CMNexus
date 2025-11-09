#pragma once
#pragma comment(lib, "ws2_32.lib")
#include<regex>
#include<string>
#include"../include/CMNetDefs.h"
#include"../include/CMThreadPool.hpp"

class AbstractIOCompletionPortClient {
public:
	AbstractIOCompletionPortClient() = default;
	~AbstractIOCompletionPortClient() = default;
	virtual bool connect(NetParms& pms) = 0;
private:
	SOCKET socket;
	std::string server_ip;
	std::string port;
};