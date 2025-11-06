#include "CMTcpServer.h"
#include "OverlappedPerIOPool.h"
#include "CommondSolve.h"
#include <random>

TcpServer::TcpServer(std::string port)
{
	WSAData data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		std::cerr << "初始化网络失败" << '\0';
		return;
	}
	else {
		std::cout << "初始化网络成功" << std::endl;
	}
	this->port = port;
	GetListenSocket() = INVALID_SOCKET;
	pms = nullptr;
	pool = new CMThreadPool(3);
	woker = new DefaultWoker;
	commondSolve = new CMCommondSolve;

	commondSolve->AddCommondsFirst("SendFile");
	commondSolve->AddCommondsFirst("GiveTotalClientId");
}

bool TcpServer::InitServer(ServerParms& pms)
{
	if ((GetListenSocket() = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)return false;
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5408);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(GetListenSocket(), (sockaddr*)&addr, sizeof(addr)) == INVALID_SOCKET) {
		closesocket(GetListenSocket());
		return false;
	}

	if (::listen(GetListenSocket(), 5) == INVALID_SOCKET) {
		closesocket(GetListenSocket());
		GetListenSocket() = INVALID_SOCKET;
		return false;
	}

	if ((GetCompletionPort() = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0)) != NULL) {
		if (CreateIoCompletionPort((HANDLE)GetListenSocket(), GetCompletionPort(), NULL, 0) != NULL) {
			pms.listen_socket = GetListenSocket();
			pms.iocp = GetCompletionPort();
			this->pms = &pms;
			return true;
		}
		CloseHandle(GetCompletionPort());
	}
	closesocket(GetListenSocket());
	return false;
}

bool TcpServer::PostAcceptEx(SOCKET& listenSocket)
{
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)return false;

	OverlappedPerIO* overlapped = new OverlappedPerIO;
	if (overlapped == nullptr) {
		closesocket(socket);
		return false;
	}

	ZeroMemory(overlapped, sizeof(OverlappedPerIO));
	overlapped->socket = socket;
	overlapped->wsaBuf.buf = overlapped->buffer;
	overlapped->wsaBuf.len = sizeof(overlapped->buffer);
	overlapped->type = IO_TYPE::IO_ACCEPT;
	overlapped->clientId = 1;
	overlapped->model = IO_MODEL::NORMAL;
	overlapped->from = FROM_INFO::FROM_CLIENT;

	DWORD byte_recv = 0;
	while (false == AcceptEx(listenSocket,
		socket,
		overlapped->wsaBuf.buf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&byte_recv,
		(LPOVERLAPPED)overlapped)) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			std::cout << "投递成功+1" << std::endl;
			break;
		}
	}
	return true;
}

/// <summary>
/// model为CUSTOM_WORK_THREAD需要额外调用addWorkThread函数
/// </summary>
/// <param name="model"></param>
/// <returns></returns>
bool TcpServer::setWorkThreadModel(const int model)
{
	if (model == DEFAULT_WORKER_THREAD) {
		if (pms->iocp == NULL || pms->listen_socket == INVALID_SOCKET) return false;
		return true;
	}
}

void TcpServer::StartPostWokerThread(const unsigned short threads)
{
	for (int i = 0; i <= threads; i++) {
		pool->enterTask([this] {
			woker->PostDefaultWokerFunction(*pms, this);
			});
	}
	pool->waitAll();
}

TcpServer::~TcpServer()
{
	if (pool != nullptr) delete pool;
	if (woker != nullptr) delete woker;
	WSACleanup();
}

void DefaultWoker::DefaultWokerFunction(ServerParms& pms, TcpServer* const server)
{
	std::cout << "工作线程启动+1" << std::endl;
	HANDLE completionPort = pms.iocp;
	SOCKET listenSocket = pms.listen_socket;

	std::random_device randomSur;
	std::mt19937 engine(randomSur());
	std::uniform_int_distribution<int> randomNumber(1, 10000);


	DWORD realBytes = 0;
	ULONG_PTR key;
	OverlappedPerIO* overlp = nullptr;
	DataHeader header;
	ReturnValueSolve returnValueSolve;
	char buffer[DEFAULT_SIZE];
	overlpPool = new OverlappedPerIOPool(10);

	std::string tempServerAgainInfor = "HelloClient";
	std::string ResourceId = "CANMENG_PERMIT_USER";

	SOCKET acceptBlockFile = INVALID_SOCKET;
	MFileSolve file_solve;
	std::string defaultTempFileName = "temp";
	while (1) {
		bool result = GetQueuedCompletionStatus(completionPort, &realBytes, &key, (LPOVERLAPPED*)&overlp, INFINITE);
		if (!result) {
			if ((GetLastError() == WAIT_TIMEOUT) || (GetLastError() == ERROR_NETNAME_DELETED)) {
				std::cout << "完成通知获取-有客户端断开连接:" << overlp->socket << std::endl;

				if (overlp->from == FROM_INFO::FROM_TEMP) {
					std::cout << "FROM_TEMP关闭+1" << std::endl;
					delete overlp;
					overlp = nullptr;
					continue;
				}
				else if (overlp->from == FROM_INFO::FROM_OVERLAPPED_POOL) {
					std::unique_lock<std::mutex> lock(server->tex);
					std::map<int, SOCKET>::iterator Key = server->clientInformation.find(overlp->clientId);
					if (Key != server->clientInformation.end()) server->clientInformation.erase(overlp->clientId);

					CancelIoEx((HANDLE)overlp->socket, (LPOVERLAPPED)overlp);
					ZeroMemory(overlp, sizeof(OverlappedPerIO));
					std::cout << "重叠结构池OVERLAPPEDPERIO回收+1" << std::endl;
					overlpPool->freeOverlapped.push(overlp);
					continue;
				}
				else if (overlp->from == FROM_INFO::FROM_CLIENT) {
					std::unique_lock<std::mutex> lock(server->tex);
					std::map<int, SOCKET>::iterator Key = server->clientInformation.find(overlp->clientId);
					if (Key != server->clientInformation.end()) {
						server->clientInformation.erase(overlp->clientId);
					}
					if (server->clientInformation.size() == 0) server->clientInformation.clear();
					std::cout << "FROM_CLIENT关闭+1" << std::endl;
					CloseClientResource(overlp);
				}
				continue;
			}
		}

		switch (overlp->type) {
		case IO_TYPE::IO_ACCEPT: {
			std::unique_lock<std::mutex> lock(server->tex);
			DefaultGetQueuedCompletionPort_AcceptCase(listenSocket, overlp, completionPort);
			while (1) {
				overlp->clientId = randomNumber(engine);
				std::map<int, SOCKET>::iterator Key = server->clientInformation.find(overlp->clientId);
				if (Key != server->clientInformation.end())continue;
				break;
			}
			server->clientInformation.insert(std::pair<int, SOCKET>(overlp->clientId, overlp->socket));
			std::cout << "----------" << '\n' << "当前连接client总数:" << server->clientInformation.size() << '\n' << "分别为:";
			for (auto& i : server->clientInformation) {
				std::cout << i.first << std::endl;
			}
		}break;
		case IO_TYPE::IO_RECV: {
			std::cout << "-----------------" << std::endl;
			std::cout << "实际接收Bytes:" << realBytes << std::endl;
			RECV_CASE result = DefaultGetQueuedCompletionPort_RecvCase(realBytes,
				overlp,
				acceptBlockFile,
				header,
				buffer,
				realBytes,
				file_solve,
				defaultTempFileName,
				server);

			switch (result) {
			case RECV_CASE::CLIENT_DATA_ERROR: { returnValueSolve.clientDataError(); }break;
			case RECV_CASE::CLIENT_ID_NONE: {
				returnValueSolve.clientIdNone();
				ReturnToAdminData(overlp, header, tempServerAgainInfor);
				continue;
			}break;
			case RECV_CASE::CLIENT_RELAY: {
				returnValueSolve.clientRelay();
				ReturnToAdminData(overlp, header, tempServerAgainInfor);
				continue;
			}break;
			case RECV_CASE::CLIENT_TO_SERVER: {
				returnValueSolve.clientToServer(overlp->buffer, realBytes, header);
				if (header.identity == 21) {
					ReturnUserClientAlive(overlp, header);
					continue;
				}
				ReturnToAdminData(overlp, header, tempServerAgainInfor);
				continue;
			}break;
			case RECV_CASE::UNKONWN_ERROR: {
				returnValueSolve.unknownError();
			}break;
			};
		}break;
		case IO_TYPE::IO_SEND: {

			if (realBytes <= 0) {
				if (WSAGetLastError() == ERROR_TIMEOUT) std::cerr << "客户端已断开:" << overlp->socket << std::endl;
				if (overlp->model == IO_MODEL::NORMAL) CloseClientResource(overlp);
				continue;
			}

			if (overlp->from == FROM_INFO::FROM_TEMP) {
				delete overlp;
				overlp = nullptr;
				std::cout << "Temp重叠结构被释放+1" << std::endl;
				continue;
			}
			else if (overlp->model == IO_MODEL::RELAY && overlp->from != FROM_INFO::FROM_TEMP) {
				ZeroMemory(overlp, sizeof(OverlappedPerIO));
				overlpPool->freeOverlapped.push(overlp);
				std::cout << "重叠结构池OVERLAPPED完成任务回收重复利用+1" << std::endl;
				continue;
			}

			ZeroMemory(overlp->buffer, sizeof(overlp->buffer));
			overlp->wsaBuf.buf = overlp->buffer;
			overlp->wsaBuf.len = sizeof(overlp->buffer);
			overlp->type = IO_TYPE::IO_RECV;
			if (overlp->model == IO_MODEL::RELAY) overlp->model = IO_MODEL::NORMAL;

			DWORD bytes = 0, flag = 0;
			WSARecv(overlp->socket, &(overlp->wsaBuf), 1, &bytes, &flag, &(overlp->overlapped), 0);
		}break;
		}
	}
}

DefaultWoker::~DefaultWoker()
{
	if (overlpPool != nullptr) {
		delete overlpPool;
		overlpPool = nullptr;
	}
}

void(DefaultWoker::* DefaultWoker::GetDefaultWokerFunction())(ServerParms&, TcpServer* server)
{
	return &DefaultWoker::DefaultWokerFunction;
}

void DefaultWoker::PostDefaultWokerFunction(ServerParms& pms, TcpServer* const server)
{
	DefaultWokerFunction(pms, server);
}

void DefaultWoker::CloseClientResource(OverlappedPerIO* overlp)
{
	if (overlp == nullptr) return;
	closesocket(overlp->socket);
	delete overlp;
	overlp = nullptr;
}

void DefaultWoker::DefaultGetQueuedCompletionPort_AcceptCase(SOCKET listenSocket, OverlappedPerIO* overlp, HANDLE completionPort)
{
	TcpServer::PostAcceptEx(listenSocket);
	setsockopt(overlp->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(listenSocket), sizeof(SOCKET));

	ZeroMemory(overlp->buffer, sizeof(overlp->buffer));
	overlp->wsaBuf.buf = overlp->buffer;
	overlp->wsaBuf.len = sizeof(overlp->buffer);
	overlp->type = IO_TYPE::IO_RECV;
	overlp->from = FROM_INFO::FROM_CLIENT;
	if (CreateIoCompletionPort((HANDLE)overlp->socket, completionPort, NULL, 0) == NULL) {
		std::cerr << "新的客户端关联完成端口失败:" << overlp->socket << std::endl;
		CloseClientResource(overlp);
	}

	std::cout << "新的客户端关联" << overlp->socket << std::endl;
	DWORD dwRecv = 0, dwFlag = 0;
	WSARecv(overlp->socket, &overlp->wsaBuf, 1, &dwRecv, &dwFlag, &(overlp->overlapped), 0);
	if (WSAGetLastError() != ERROR_IO_PENDING) {
		std::cerr << "客户端在发送数据时断开连接:" << overlp->socket << std::endl;
		CloseClientResource(overlp);
		return;
	}
}

RECV_CASE DefaultWoker::DefaultGetQueuedCompletionPort_RecvCase(DWORD realBytes,
	OverlappedPerIO* overlp,
	SOCKET acceptBlockFile,
	DataHeader& header,
	char* buffer,
	const int bufferLen,
	MFileSolve& fileSolve, std::string& defaultTempFileName,
	TcpServer* server)
{
	if (realBytes <= 0) {
		std::cerr << "接收数据为0，客户端断开连接:" << overlp->socket << std::endl;
		CloseClientResource(overlp);
		return RECV_CASE::CLIENT_TIMEOUT;
	}
	else if (realBytes < sizeof(DataHeader)) {
		std::cerr << "客户端传输数据有误，请及时检查" << std::endl;
		return RECV_CASE::CLIENT_TIMEOUT;
	}

	if (acceptBlockFile == INVALID_SOCKET && acceptBlockFile != overlp->socket) acceptBlockFile = overlp->socket;

	DataHeaderInit::ZeroMemoryDataHeader(&header);
	memset(buffer, 0, bufferLen);
	memcpy(&header, overlp->buffer, sizeof(DataHeader));

	DataHeaderInit::NetworkToHostDataHeader(header);
	//转发及逻辑
	if (header.targetClientId == 0) return RECV_CASE::CLIENT_TO_SERVER;
	if (header.PresentConduct == 2 && header.isRelay == 0) {

		DWORD bytes = 0, flag = 0;
		std::map<int, SOCKET>::iterator Key;
		{
			std::unique_lock<std::mutex>  lock(server->tex);
			Key = server->clientInformation.find(header.targetClientId);
			if (!(Key != server->clientInformation.end())) return RECV_CASE::CLIENT_ID_NONE;
		}
		if (overlpPool->freeOverlapped.empty()) {
			OverlappedPerIO* overl = new OverlappedPerIO;
			ZeroMemory(overl, sizeof(OverlappedPerIO));

			overl->model = IO_MODEL::RELAY;
			overl->socket = Key->second;
			overl->clientId = -1;
			overl->type = IO_TYPE::IO_SEND;
			overl->wsaBuf.buf = overl->buffer;
			overl->wsaBuf.len = bufferLen;
			overl->from = FROM_INFO::FROM_TEMP;

			memcpy(overl->buffer, overlp->buffer, sizeof(overlp->buffer));

			DWORD bytes = 0, flag = 0;
			WSASend(Key->second, &(overl->wsaBuf), 1, &bytes, flag, &(overl->overlapped), 0);
			std::cout << "成功将数据转发到:" << Key->second << std::endl;
			return RECV_CASE::CLIENT_RELAY;
		}
		overlpPool->OverlappedSendInfo(overlp->buffer, sizeof(overlp->buffer), 1, Key->second);
		return RECV_CASE::CLIENT_RELAY;
	}
}

void DefaultWoker::ReturnToAdminData(OverlappedPerIO* overlp, DataHeader& header, std::string tempServerAgainInfor)
{
	memset(overlp->buffer, 0, sizeof(overlp->buffer));
	memset(&header, 0, sizeof(DataHeader));


	header.PresentConduct = htons(2);
	header.dataType = htons(4);
	header.presentDataBytes = htons(tempServerAgainInfor.size());
	header.presentChunksIndex = htons(0);
	header.isRelay = htons(1);
	header.targetClientId = htons(overlp->clientId);
	header.totalChunks = htons(1);
	header.totalDataBytes = htonl(tempServerAgainInfor.size());

	memcpy(overlp->buffer, &header, sizeof(DataHeader));
	memcpy(overlp->buffer + sizeof(DataHeader), tempServerAgainInfor.data(), tempServerAgainInfor.size());

	overlp->wsaBuf.buf = overlp->buffer;
	overlp->wsaBuf.len = sizeof(overlp->buffer);
	overlp->type = IO_TYPE::IO_SEND;

	DWORD byte = 0;
	WSASend(overlp->socket, &(overlp->wsaBuf), 1, &byte, 0, &(overlp->overlapped), 0);
	std::cout << "成功发送回信" << std::endl;
}

void DefaultWoker::ReturnUserClientAlive(OverlappedPerIO* overlp, DataHeader& header)
{
	memset(&header, 0, sizeof(DataHeader));
	ZeroMemory(overlp->buffer, sizeof(overlp->buffer));

	header.identity = htons(0);
	header.PresentConduct = htons(4);

	memcpy(overlp->buffer, &header, sizeof(DataHeader));
	overlp->wsaBuf.buf = overlp->buffer;
	overlp->wsaBuf.len = sizeof(DataHeader);
	overlp->type = IO_TYPE::IO_SEND;

	DWORD byte = 0;
	WSASend(overlp->socket, &(overlp->wsaBuf), 1, &byte, 0, &(overlp->overlapped), 0);
	std::cout << "成功回应用户客户端数据" << std::endl;
}

void ReturnValueSolve::clientDataError()
{
	std::cout << "客户端发送数据发生错误或数据本身具有错误..." << std::endl;
}

void ReturnValueSolve::clientIdNone()
{
	std::cout << "未知目标客户端id，已停止转发..." << std::endl;
}

void ReturnValueSolve::clientRelay()
{
	std::cout << "<客户端发送数据转发提示>" << std::endl;
}

void ReturnValueSolve::clientToServer(const char* buffer, int len, DataHeader& header)
{
	std::cout << "-------------" << '\n' << "客户端->服务端" << "发送类型为:ONLY_STRING" << '\n' << "数据实际总字节数:" << header.totalDataBytes << std::endl;
}

void ReturnValueSolve::normal() {}

void ReturnValueSolve::unknownError()
{
}
