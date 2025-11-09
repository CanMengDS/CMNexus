#include "../include/utils/DataHeaderInit.h"
#include <WinSock2.h>

void DataHeaderInit::ZeroMemoryDataHeader(DataHeader* header)
{
	memset(header, 0, sizeof(DataHeader));
}

void DataHeaderInit::NetworkToHostDataHeader(DataHeader& header)
{
	header.isRelay = ntohs(header.isRelay);
	header.targetClientId = ntohs(header.targetClientId);
	header.totalDataBytes = ntohl(header.totalDataBytes);
	header.presentDataBytes = ntohs(header.presentDataBytes);
	header.totalChunks = ntohs(header.totalChunks);
	header.presentChunksIndex = ntohs(header.presentChunksIndex);
	header.dataType = ntohs(header.dataType);
	header.PresentConduct = ntohs(header.PresentConduct);
	header.identity = ntohs(header.identity);
}

void DataHeaderInit::HostToNetworkDataHeader(DataHeader& header, std::string& message)
{
	header.presentDataBytes = htons(message.size());
	header.presentChunksIndex = htons(0);
	header.totalDataBytes = htonl(message.size());
	header.totalChunks = htons(1);
	header.dataType = htons(3);
	header.PresentConduct = htons(2);
	header.targetClientId = htons(0);
}
