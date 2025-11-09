#pragma once
#include<iostream>
#include "../include/CMNetDefs.h"

class DataHeaderInit {
public:
	DataHeaderInit() = default;
	~DataHeaderInit() = default;
	static void ZeroMemoryDataHeader(DataHeader* header);
	static void NetworkToHostDataHeader(DataHeader& header);
	static void HostToNetworkDataHeader(DataHeader& header, std::string& message); //传入参数无需网络字节序化
};