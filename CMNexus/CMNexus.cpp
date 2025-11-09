// CMNexus.cpp: 定义应用程序的入口点。
//
#include "../include/CMTcpServer.h"
#include "../include/CMThreadPool.hpp"
#include "../include/CMNetDefs.h"
#define DEFAULT_SIZE 1024

int main() {
	TcpServer server("5408");
	NetParms pms;
	if (!server.InitServer(pms)) {
		std::cerr << "初始化服务端失败" << '\n';
		return 666;
	}
	else {
		std::cout << "初始化服务端成功" << std::endl;
	}

	for (int i = 0; i <= 3; i++) server.PostAcceptEx(pms.socket);

	CMThreadPool pool(3);
	server.setWorkThreadModel(DEFAULT_WORKER_THREAD);
	server.StartPostWokerThread(3);
}