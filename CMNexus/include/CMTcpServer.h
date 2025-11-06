#pragma once
#include "AbstractIOCompletionPortServer.h"
#include <map>
#include "CMNetDefs.h"
#include "CMThreadPool.hpp"
#include "CMFileSolve.h"
#include "DataHeaderInit.h"
#include "CommondSolve.h"

#define DEFAULT_WORKER_THREAD 0
#define CUSTOM_WORKER_THREAD 1

struct ServerParms;
class DefaultWoker; 
class OverlappedPerIOPool;

class TcpServer : public AbstractIOCompletionPortServer {
	friend class DefaultWoker;
public:
	TcpServer(std::string port);
	bool InitServer(ServerParms& pms);
	static bool PostAcceptEx(SOCKET& listenSocket);
	bool setWorkThreadModel(const int model);
	void StartPostWokerThread(const unsigned short threads);
	~TcpServer();
	std::string port;
private:
	CMThreadPool* pool;
	DefaultWoker* woker;
	ServerParms* pms;
	CMCommondSolve* commondSolve;
	std::map<int, SOCKET> clientInformation;
	std::mutex tex;
};

class DefaultWoker {
public:
	DefaultWoker() = default;
	~DefaultWoker();
	void(DefaultWoker::* GetDefaultWokerFunction())(ServerParms&, TcpServer* server);
	void PostDefaultWokerFunction(ServerParms& pms, TcpServer* const server);
private:

	void DefaultWokerFunction(ServerParms& pms, TcpServer* const server);
	void CloseClientResource(OverlappedPerIO* overlp);
	void DefaultGetQueuedCompletionPort_AcceptCase(SOCKET listenSocket, OverlappedPerIO* overlp, HANDLE completionPort);
	RECV_CASE DefaultGetQueuedCompletionPort_RecvCase(DWORD realBytes,
		OverlappedPerIO* overlp,
		SOCKET acceptBlockFile,
		DataHeader& header,
		char* buffer,
		const int bufferLen,
		MFileSolve& file_solve,
		std::string& defaultTempFileName,
		TcpServer* server);
	void ReturnToAdminData(OverlappedPerIO* overlp, DataHeader& header, std::string tempServerAgainInfor);
	void ReturnUserClientAlive(OverlappedPerIO* overlp, DataHeader& header);

	OverlappedPerIOPool* overlpPool = nullptr;
};

class ReturnValueSolve {
public:
	ReturnValueSolve() = default;
	~ReturnValueSolve() = default;

	void clientDataError();
	void clientIdNone();
	void clientRelay();
	void clientToServer(const char* buffer, int len, DataHeader& header);
	void normal();
	void unknownError();
};

