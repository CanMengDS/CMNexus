#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#define DEFAULT_SIZE 1024
//#pragma pack(push, 1)

struct DataHeader {
	uint16_t isRelay; //0则转发，1则不转
	uint16_t targetClientId;
	//uint16_t presentDataId; //当连续发送时接收端(通常为目标客户端，因为服务端仅负责转发，识别操作为目标客户端负责)用于检测是否和上一个数据是同样的id，如果是则无误，如果不是则有错误
	uint16_t totalChunks;
	uint32_t totalDataBytes;
	uint16_t presentChunksIndex;
	uint16_t presentDataBytes;
	/*
	1:TXT
	2:ZIP
	3:JPG
	4:ONLY_STRING
	5:SSYTEM_COMMOND
	*/
	uint16_t dataType;
	/*
	* 1:APPLY_FOR_TOTAL_CLIENT_ID
	* 2:SEND_DATA
	* 3:RECV_DATA
	* 4.ALIVE_CHECK
	*/
	uint16_t PresentConduct;
	/*
	5408:admin
	21:user
	0:server
	*/
	uint16_t identity;
};

enum class RECV_CASE {
	CLIENT_ID_NONE,
	CLIENT_TIMEOUT,
	CLIENT_DATA_ERROR,
	CLIENT_RELAY,
	CLIENT_TO_SERVER,
	UNKONWN_ERROR
};

struct NetParms {
	SOCKET socket;
	HANDLE iocp;
};

enum class IO_TYPE {
	IO_ACCEPT,
	IO_SEND,
	IO_RECV,
	IO_CONNECT,
	IO_DISCONNECT
};

enum class IO_MODEL {
	NORMAL,
	RELAY
};

enum class FROM_INFO {
	FROM_OVERLAPPED_POOL,
	FROM_TEMP,
	FROM_CLIENT
};

typedef struct OverlappedPerIO {
	OVERLAPPED overlapped;
	SOCKET socket;
	WSABUF wsaBuf;
	IO_TYPE type;
	IO_MODEL model;
	FROM_INFO from;
	int clientId; //每一个client对应一个OverlappedPerIO，lientIdIndex则是对应了当前OverlappedPerIO对应的client的clientId，方便资源回收时通过overlp找到对应SOCKET进行清理
	char buffer[DEFAULT_SIZE];
};