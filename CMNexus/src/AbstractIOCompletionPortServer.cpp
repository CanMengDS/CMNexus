#include "../include/Abstract/AbstractIOCompletionPortServer.h"
#include"../include/CMNetDefs.h"
bool AbstractIOCompletionPortServer::PostAcceptEx(SOCKET& listenSocket)
{
    return false;
}

SOCKET& AbstractIOCompletionPortServer::GetListenSocket()
{
    return listenSocket;
}

HANDLE& AbstractIOCompletionPortServer::GetCompletionPort()
{
    return iocp;
}
