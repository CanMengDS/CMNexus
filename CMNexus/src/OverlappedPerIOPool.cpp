#include "../include/OverlappedPerIOPool.h"
#include "../include/CMNetDefs.h"

OverlappedPerIOPool::OverlappedPerIOPool(const int overlappedPerIONum)
{
	for (int i = 0; i < overlappedPerIONum; i++) {
		OverlappedPerIO* overlp = new OverlappedPerIO;
		ZeroMemory(overlp, sizeof(OverlappedPerIO));
		overlp->from = FROM_INFO::FROM_OVERLAPPED_POOL;
		overlappeds.push_back(overlp);
		freeOverlapped.push(overlp);
	}
}

bool OverlappedPerIOPool::OverlappedSendInfo(const char* buffer, const int len, int targetID, const SOCKET target)
{
	std::unique_lock<std::mutex> lock(tex);
	if (freeOverlapped.empty()) return false;
	OverlappedPerIO* overlp = freeOverlapped.front();
	overlp->type = IO_TYPE::IO_SEND;
	memcpy(overlp->buffer, buffer, len);
	overlp->clientId = targetID;
	overlp->model = IO_MODEL::RELAY;
	overlp->socket = target;
	overlp->wsaBuf.buf = overlp->buffer;
	overlp->wsaBuf.len = len;
	overlp->from = FROM_INFO::FROM_OVERLAPPED_POOL;

	DWORD bytes = 0, flag = 0;
	bool result = WSASend(overlp->socket, &(overlp->wsaBuf), 1, &bytes, flag, &(overlp->overlapped), 0);
	if (!result) {
		if (GetLastError() == SOCKET_ERROR && GetLastError() != ERROR_IO_PENDING) {
			std::cerr << "发送数据失败，错误来自:" << WSAGetLastError() << std::endl;
			return false;
		}
	}
	std::cout << "成功将数据转发到:" << target << std::endl;
	freeOverlapped.pop();
	return true;
}

OverlappedPerIOPool::~OverlappedPerIOPool()
{
	for (auto* i : overlappeds) {
		if (i != nullptr) {
			delete i;
			i = nullptr;
		}
	}
}
