#pragma once
#include<vector>
#include<queue>
#include<mutex>
#include<iostream>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

typedef struct OverlappedPerIO;

class OverlappedPerIOPool {
public:
	OverlappedPerIOPool(const int overlappedPerIONum);
	bool OverlappedSendInfo(const char* buffer, const int len, int targetID, const SOCKET target);
	~OverlappedPerIOPool();

	std::queue<OverlappedPerIO*> freeOverlapped;
private:
	std::vector<OverlappedPerIO*> overlappeds;
	std::mutex tex;
};