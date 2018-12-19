#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include <vector>
#include <thread>
#include <string>

#define SERVERPORT 9000
#define BUFSIZE    512

#pragma warning (disable : 4996)

enum PACKET_TYPE { TYPE_LOGIN, TYPE_DATA, };

VOID ProcessPacket(char* _buffer, PACKET_TYPE* _type, char* _data, int _bufferLength);
VOID PackingPacket(char* _buffer, PACKET_TYPE* _type, char* _data, int* _bufferLength);

class ChatServer
{
private:
	WSADATA wsa;

	SOCKET listenSock;
	SOCKADDR_IN serverAddr;

	SOCKET clientSock;
	SOCKADDR_IN clientAddr;

	int addrLen;

	std::vector<SOCKET> clientSockList;

	CRITICAL_SECTION criticalSection;

	HANDLE event;

public:
	ChatServer();
	~ChatServer();

	BOOL Socket();
	BOOL Bind(USHORT _port);
	BOOL Listen(INT _backLog);

	VOID AcceptLoop();

	BOOL PROC_PACKET_LOGIN(SOCKET* _sock, std::string& _id, char* _data);
	BOOL PROC_PACKET_DATA(SOCKET* _sock, std::string& _id, char* _data);

	BOOL WriteAllPacket(SOCKET* _sock, std::string _message, bool _notOwner);

	DWORD ProcessClient(SOCKET _sock);
};

