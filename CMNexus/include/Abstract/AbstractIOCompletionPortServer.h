#pragma once
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include<iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

struct NetParms;

class AbstractIOCompletionPortServer {
public:
	AbstractIOCompletionPortServer() = default;
	~AbstractIOCompletionPortServer() = default;
	virtual bool InitServer(NetParms& pms) = 0;
	static bool PostAcceptEx(SOCKET& listenSocket);
	SOCKET& GetListenSocket();
	HANDLE& GetCompletionPort();
private:
	HANDLE iocp;
	SOCKET listenSocket;
};