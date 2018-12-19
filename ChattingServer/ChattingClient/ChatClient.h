#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    50

#pragma warning (disable : 4996)

enum PACKET_TYPE { TYPE_LOGIN, TYPE_DATA, };

VOID ProcessPacket(char* _buffer, PACKET_TYPE* _type, char* _data, int _bufferLength);
VOID PackingPacket(char* _buffer, PACKET_TYPE* _type, char* _data, int* _bufferLength);

class ChatClient
{
private:
	WSADATA wsa;

	SOCKET sock;
	SOCKADDR_IN serveraddr;

	std::thread* workThread;

	VOID ReceiveLoop();

public:
	ChatClient();
	~ChatClient();

	BOOL Socket();
	bool Connect(std::string _ip, USHORT _port);

	VOID Loop();
};

