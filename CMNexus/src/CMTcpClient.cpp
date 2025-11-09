#pragma once
#include "../include/CMTcpClient.h"

Client::Client(std::string serverIP, std::string port)
{
	WSAData data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		std::cerr << "初始化网络错误" << std::endl;
		return;
	}
	this->serverIP = serverIP;
	this->port = port;
	socket = INVALID_SOCKET;

	pool = new CMThreadPool(3);
	defaultWoker = new DefaultWoker;
	net_data = new CMNetData;
	pms = nullptr;

	commondSolve = new CMCommondSolve;
	commondSolve->AddCommondsFirst("SendFile", [](DataHeader* header, int target) {
		std::cout << "holyShit" << std::endl;
		header->targetClientId = htons(target);
		header->PresentConduct = htons(2);
		});

	commondSolve->AddCommondsFirst("GiveTotalClientId", [](DataHeader* header, int target) {
		header->targetClientId = htons(target);
		header->PresentConduct = htons(1);
		});

	net_data->AddCMDataHeaderReturnDataCaseAndSolveFunction(1, [](char* buffer, int len) {
		DataHeader header;
		ZeroMemory(&header, sizeof(DataHeader));
		memcpy(&header, buffer, sizeof(DataHeader));
		header.presentDataBytes = ntohs(header.presentDataBytes);

		std::vector<int> client_id(header.presentDataBytes / sizeof(int));
		memcpy(client_id.data(), buffer + sizeof(DataHeader), header.presentDataBytes);
		std::cout << "---------------------";
		for (auto& e : client_id) {
			std::cout << e << std::endl;
		}
		std::cout << "------------------------";
		});
}

bool Client::connect(NetParms& pms)
{
	if ((pms.iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0)) == NULL) {
		std::cerr << "创建完成端口失败" << std::endl;
		return false;
	}

	if (socket != INVALID_SOCKET)return false;

	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* addr;
	if (::getaddrinfo(serverIP.data(), port.data(), &hints, &addr) != 0) return false;

	for (; addr != nullptr; addr = addr->ai_next) {
		if ((socket = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == INVALID_SOCKET) return false;
		if (::connect(socket, addr->ai_addr, addr->ai_addrlen) == 0) break;

		closesocket(socket);
		continue;
	}

	if (CreateIoCompletionPort((HANDLE)socket, pms.iocp, NULL, 0) != NULL) {
		pms.socket = socket;
		std::clog << "成功创建完成端口" << std::endl;
		this->pms = &pms;
		return true;
	}
	std::cerr << "关联iocp或创建socket失败" << std::endl;
	closesocket(socket);
	return false;
}

void Client::setWorkerThreadModel(const int model)
{
	if (model == DEFAULT_WORKER_THREAD_MODEL) {
		std::string buff;
		std::string message = "HelloWorld";
		DataHeader header;
		memset(&header, 0, sizeof(header));
		header.presentDataBytes = htons(message.size());
		header.presentChunksIndex = htons(0);
		header.totalDataBytes = htonl(message.size());
		header.totalChunks = htons(1);
		header.dataType = htons(3);
		header.PresentConduct = htons(2);
		header.targetClientId = htons(0);
		header.isRelay = htons(1);
		header.identity = htons(5408);

		OverlappedPerIO* overlp = new OverlappedPerIO;
		ZeroMemory(overlp, sizeof(OverlappedPerIO));
		overlp->socket = pms->socket;
		overlp->wsaBuf.buf = overlp->buffer;
		overlp->wsaBuf.len = sizeof(DataHeader) + message.size();
		overlp->type = IO_TYPE::IO_SEND;
		memcpy(overlp->buffer, &header, sizeof(DataHeader));
		memcpy(overlp->buffer + sizeof(DataHeader), message.c_str(), message.size());

		DWORD byte = 0;
		WSASend(overlp->socket, &(overlp->wsaBuf), 1, &byte, 0, &overlp->overlapped, 0);


		std::thread t([this] {
			this->defaultWoker->DefaultWokerFunction(pms, this);
			});

		t.join();
	}
}

DefaultWoker* Client::GetDefaultWoekr()
{
	return defaultWoker;
}

Client::~Client()
{
}

void DefaultWoker::DefaultWokerFunction(NetParms* pms, Client* client)
{
	SOCKET socket = pms->socket;
	HANDLE iocp = pms->iocp;

	OverlappedPerIO* overlp = nullptr;
	DWORD realBytes = 0;
	ULONG_PTR flag = 0;
	char buffer[1024];
	int targetClient = -1, send_num = 0;

	std::string message = "HelloTargetClient";
	std::string path = "D:\\pptm.txt";

	CMFileSolve fileSolve;

	DataHeader header;
	memset(&header, 0, sizeof(header));

	bool isTransmissionBlockFile = false;
	int realDataSize = sizeof(buffer) - sizeof(DataHeader); //1008

	bool isBlockFileTrans = false;
	int presentEnd = 0;

	int transNum = 0;
	while (1) {
		bool result = GetQueuedCompletionStatus(iocp, &realBytes, &flag, (LPOVERLAPPED*)&overlp, INFINITE);
		if (!result) {
			if (WSAGetLastError() == WAIT_TIMEOUT || WSAGetLastError() == ERROR_NETNAME_DELETED) {
				std::cerr << "服务端断开连接" << std::endl;
				closesocket(socket);
				delete overlp;
				overlp = nullptr;
				break;
			}
		}

		switch (overlp->type) {
		case IO_TYPE::IO_RECV: {
			std::cout << "真实服务端发送数据量:" << realBytes << std::endl;
			if (realBytes <= 0) {
				std::cerr << "服务端未知原因断开" << std::endl;
				closesocket(socket);
				delete overlp;
				break;
			}
			else if (realBytes < sizeof(DataHeader)) {
				std::cerr << "服务端发送数据有误，请及时检查" << std::endl;
				continue;
			}
			std::cout << "-----------------" << '\n' << "进入服务端返回数据处理阶段" << std::endl;
			memset(&header, 0, sizeof(DataHeader));
			memcpy(&header, overlp->buffer, realBytes);

			header.PresentConduct = ntohs(header.PresentConduct);
			header.dataType = ntohs(header.dataType);
			header.identity = ntohs(header.identity);
			header.presentDataBytes = ntohs(header.presentDataBytes);

			client->net_data->CheckDataCaseAndRunSolveFunction(header, overlp->buffer, realBytes);
			std::cout << "--------------------" << "服务端数据处理结束" << std::endl;

			GetQueuedCompletionStatusIoRecvCaseUserInputCommond(client->commondSolve,
				isBlockFileTrans,
				transNum,
				header,
				message,
				overlp,
				socket,
				fileSolve,
				buffer,
				result,
				presentEnd
			);
		}break;
		case IO_TYPE::IO_SEND: {


			if (realBytes <= 0) {
				std::cerr << "服务端断开连接" << std::endl;
				closesocket(overlp->socket);
				break;
			}
			transNum++;
			std::cout << "发送成功，成功发送数据:" << realBytes << '\n' << "已发送分块数:" << transNum << '\n' << "--------------------------------" << std::endl;
			int result = -1;


			send_num++;

			memset(&header, 0, sizeof(DataHeader));
			ZeroMemory(overlp->buffer, sizeof(overlp->buffer));
			overlp->type = IO_TYPE::IO_RECV;
			overlp->wsaBuf.buf = overlp->buffer;
			overlp->wsaBuf.len = sizeof(overlp->buffer);

			DWORD byte = 0, flag = 0;
			WSARecv(overlp->socket, &(overlp->wsaBuf), 1, &byte, &flag, &(overlp->overlapped), 0);
			std::cout << "成功投递WSARecv" << std::endl;
		}break;
		}
	}
}

void DefaultWoker::GetQueuedCompletionStatusIoRecvCaseUserInputCommond(
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
)
{
	std::string userResult;
	while (1) {
		if (!isTransmissionBlockFile) {
			transNum = 0;
			std::cout << "请输入指令..." << std::endl;
			std::getline(std::cin, userResult);
			std::vector<std::string> commondPart;
			bool result = commondSolve->GetCommondGroup(userResult, commondPart);
			if (!result) continue;
			for (auto& commonds : commondSolve->GetCommondsVector()) {
				if (commondPart[0].compare(commonds.commond) == 0) {
					commonds.solveFunction(&header, stoi(commondPart[1]));
					break;
				}
			}

			std::copy(message.begin(), message.end(), overlp->buffer);
			header.presentDataBytes = htons(message.size());
			header.presentChunksIndex = htons(0);
			header.totalDataBytes = htonl(message.size());
			header.totalChunks = htons(1);
			header.dataType = htons(3);
			header.isRelay = htons(0);
			header.identity = htons(5408);
			ZeroMemory(overlp->buffer, sizeof(overlp->buffer));
			overlp->wsaBuf.buf = overlp->buffer;
			overlp->wsaBuf.len = sizeof(DataHeader) + ntohs(header.presentDataBytes);
			overlp->type = IO_TYPE::IO_SEND;

			memcpy(overlp->buffer, &header, sizeof(DataHeader));
			memcpy(overlp->buffer + sizeof(DataHeader), message.data(), ntohs(header.presentDataBytes));

			DWORD byte = 0, flag = 0;
			WSASend(socket, &overlp->wsaBuf, 1, &byte, flag, &overlp->overlapped, 0);
			break;
		}
		int totalSize = fileSolve.GetTotalSize("D:\\Desktop\\QQ20251029-212129.png");

		ZeroMemory(buffer, sizeof(buffer));
		result = fileSolve.ReadFile("D:\\Desktop\\QQ20251029-212129.png", buffer, 1024 - sizeof(DataHeader));
		presentEnd++;

		header.PresentConduct = htons(2);
		header.dataType = htons(3);
		header.isRelay = htons(0);
		header.presentChunksIndex = htons(presentEnd);
		header.presentDataBytes = htons(result);
		header.targetClientId = htons(2);
		header.totalChunks = htons(13);
		header.totalDataBytes = htonl(totalSize);
		header.identity = htons(5408);

		ZeroMemory(overlp->buffer, sizeof(overlp->buffer));
		overlp->wsaBuf.buf = overlp->buffer;
		overlp->wsaBuf.len = sizeof(DataHeader) + ntohs(header.presentDataBytes);
		overlp->type = IO_TYPE::IO_SEND;

		memcpy(overlp->buffer, &header, sizeof(DataHeader));
		memcpy(overlp->buffer + sizeof(DataHeader), buffer, result);

		DWORD byte = 0, flag = 0;
		WSASend(socket, &overlp->wsaBuf, 1, &byte, flag, &overlp->overlapped, 0);
		if (result < 1024 - sizeof(DataHeader)) {
			std::cout << "连续传输完成" << std::endl;
			isTransmissionBlockFile = false;
			presentEnd = 0;
		}
		break;
	}
}


