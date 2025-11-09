#include "../include/CMNetDefs.h"
#include "../include/CMThreadPool.hpp"
#include "../include/utils/CMFileSolve.h"
#include "../include/utils/CommondSolve.h"
#include "../include/utils/CMNetData.h"
#include "../include/utils/DataHeaderInit.h"
#define DEFAULT_WORKER_THREAD_MODEL 0
#define CUSTOM_WORKER_THREAD_MODEL 1

class DefaultWoker;

class Client {
	friend class DefaultWoker;
public:
	Client(std::string serverIP, std::string port);
	bool connect(NetParms& pms);
	void setWorkerThreadModel(const int model);
	DefaultWoker* GetDefaultWoekr();
	~Client();

private:
	SOCKET socket;
	std::string serverIP;
	std::string port;

	CMThreadPool* pool;
	NetParms* pms;
	CMCommondSolve* commondSolve;
	CMFileSolve solve;
	CMNetData* net_data;
	DefaultWoker* defaultWoker;
};

class DefaultWoker {
public:
	DefaultWoker() = default;
	~DefaultWoker() = default;
	void DefaultWokerFunction(NetParms* pms, Client* client);
private:

	void GetQueuedCompletionStatusIoRecvCaseUserInputCommond(
		CMCommondSolve* commondSolve,
		bool isTransmissionBlockFile,
		int transNum,
		DataHeader& header,
		std::string message,
		OverlappedPerIO* overlp,
		SOCKET socket,
		CMFileSolve& fileSolve,
		char* buffer,
		bool result,
		int presentEnd
	);
};