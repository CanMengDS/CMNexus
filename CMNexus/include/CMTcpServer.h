#pragma once
#include "../include/Abstract/AbstractIOCompletionPortServer.h"
#include <map>
#include "../include/CMNetDefs.h"
#include "../include/CMThreadPool.hpp"
#include "../include/utils/CMFileSolve.h"
#include "../include/utils/DataHeaderInit.h"
#include "../include/utils/CommondSolve.h"

#define DEFAULT_WORKER_THREAD 0
#define CUSTOM_WORKER_THREAD 1

struct ServerParms;
class DefaultWoker; 
class OverlappedPerIOPool;

class TcpServer : public AbstractIOCompletionPortServer {
	friend class DefaultWoker;
public:
	TcpServer(std::string port);
	bool InitServer(NetParms& pms);
	static bool PostAcceptEx(SOCKET& listenSocket);
	bool setWorkThreadModel(const int model);
	void StartPostWokerThread(const unsigned short threads);
	~TcpServer();
	std::string port;
private:
	CMThreadPool* pool;
	DefaultWoker* woker;
	NetParms* pms;
	CMCommondSolve* commondSolve;
	std::map<int, SOCKET> clientInformation;
	std::mutex tex;
};

class DefaultWoker {
public:
	DefaultWoker() = default;
	~DefaultWoker();
	void(DefaultWoker::* GetDefaultWokerFunction())(NetParms&, TcpServer* server);
	void PostDefaultWokerFunction(NetParms& pms, TcpServer* const server);
private:

	void DefaultWokerFunction(NetParms& pms, TcpServer* const server);
	void CloseClientResource(OverlappedPerIO* overlp);
	void DefaultGetQueuedCompletionPort_AcceptCase(SOCKET listenSocket, OverlappedPerIO* overlp, HANDLE completionPort);
	RECV_CASE DefaultGetQueuedCompletionPort_RecvCase(DWORD realBytes,
		OverlappedPerIO* overlp,
		SOCKET acceptBlockFile,
		DataHeader& header,
		char* buffer,
		const int bufferLen,
		CMFileSolve& file_solve,
		std::string& defaultTempFileName,
		TcpServer* server);
	void ReturnToAdminData(OverlappedPerIO* overlp, DataHeader& header, std::string tempServerAgainInfor);
	void ReturnUserClientAlive(OverlappedPerIO* overlp, DataHeader& header);
	void ReturnTotalClientId(OverlappedPerIO* overlp, const TcpServer* server, DataHeader& header, std::vector<int>& client_id);
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

