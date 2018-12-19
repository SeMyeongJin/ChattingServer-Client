#include "ChatServer.h"



VOID ProcessPacket(char* _buffer, PACKET_TYPE* _type, char* _data, int _bufferLength)
{
	CopyMemory(
		_type,
		_buffer, sizeof(PACKET_TYPE)
	);

	CopyMemory(
		_data,
		_buffer + sizeof(PACKET_TYPE),
		_bufferLength - sizeof(PACKET_TYPE)
	);
}

VOID PackingPacket(char* _buffer, PACKET_TYPE* _type, char* _data, int* _bufferLength)
{
	CopyMemory(
		_buffer,
		_type,
		sizeof(PACKET_TYPE)
	);

	CopyMemory(
		_buffer + sizeof(PACKET_TYPE),
		_data,
		strlen(_data) + 1
	);

	*_bufferLength = sizeof(PACKET_TYPE) + strlen(_data) + 1;
}

ChatServer::ChatServer()
{
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	InitializeCriticalSection(&criticalSection);

	addrLen = 0;
}

ChatServer::~ChatServer()
{
	if (listenSock == NULL)
		closesocket(listenSock);

	DeleteCriticalSection(&criticalSection);

	WSACleanup();
}

BOOL ChatServer::Socket()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return FALSE;

	listenSock = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSock == INVALID_SOCKET)
	{
		return FALSE;
	}

	u_long on = 1;
	int retval = ioctlsocket(listenSock, FIONBIO, &on);

	if (retval == SOCKET_ERROR) printf("Socket Error!\n");

	printf("socket() completely!\n");

	return TRUE;
}

BOOL ChatServer::Bind(USHORT _port)
{
	serverAddr.sin_port = htons(_port);

	int retval;

	retval = bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	if (retval == SOCKET_ERROR)
	{
		printf("Bind Error!\n");
		return FALSE;
	}

	printf("bind() completely!\n");

	return TRUE;
}

BOOL ChatServer::Listen(INT _backLog)
{
	int retval;

	retval = listen(listenSock, _backLog);
	if (retval == SOCKET_ERROR)
	{
		printf("listen Error");
		return FALSE;
	}

	printf("listen() completely!\n");

	return TRUE;
}

VOID ChatServer::AcceptLoop()
{
	printf("accept() completely!\n");

	printf("========================== Chatting Server Start! ==========================\n");

	while (TRUE)
	{
		do
		{
			addrLen = sizeof(clientAddr);

			clientSock = accept(listenSock, (SOCKADDR *)&clientAddr, &addrLen);

			if (clientSock == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
					continue;

				printf("Accept Error!\n");
				return;
			}

			break;

		} while (TRUE);

		printf("[client connected] : IP = %s, Port = %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		std::thread* workThread = new std::thread(&ChatServer::ProcessClient, this, clientSock);
	}
}

DWORD ChatServer::ProcessClient(SOCKET _sock)
{
	SOCKET sock = _sock;

	int retval;
	SOCKADDR_IN clientAddr;
	int addrLen;
	char buf[BUFSIZE + 1];

	addrLen = sizeof(clientAddr);
	getpeername(sock, (SOCKADDR *)&clientAddr, &addrLen);

	bool flag = TRUE;
	std::string id = "";
	std::string message = "";

	while (flag)
	{
		EnterCriticalSection(&criticalSection);

		retval = recv(sock, buf, BUFSIZE, 0);

		LeaveCriticalSection(&criticalSection);

		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			flag = FALSE;
			break;
		}

		buf[retval] = '\0';

		PACKET_TYPE type;
		char data[BUFSIZE + 1];
		ProcessPacket(buf, &type, data, retval);

		switch (type) {
		case TYPE_LOGIN:
			if (!PROC_PACKET_LOGIN(&sock, id, data))
				flag = false;
			break;

		case TYPE_DATA:
			if (!PROC_PACKET_DATA(&sock, id, data))
				flag = false;
			break;
		}
	}

	message = "Leave user (" + id + ")";

	WriteAllPacket(&sock, message, true);

	closesocket(sock);

	printf("[client Disconnected] : IP = %s, Port = %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

	return 0;
}

BOOL ChatServer::WriteAllPacket(SOCKET* _sock, std::string _message, bool _eraseOwner)
{
	EnterCriticalSection(&criticalSection);

	for (auto iter = clientSockList.begin(); iter != clientSockList.end();)
	{
		if (*iter == *_sock && _eraseOwner)
		{
			iter = clientSockList.erase(iter);

			continue;
		}
		else
		{
			while (true)
			{
				int retval = send(*iter, _message.c_str(), _message.size() + 1, 0);

				if (retval == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
						continue;
					break;

					return false;
				}

				break;
			}

			iter++;
		}
	}

	LeaveCriticalSection(&criticalSection);

	return true;
}

BOOL ChatServer::PROC_PACKET_LOGIN(SOCKET* _sock, std::string& _id, char* _data)
{
	EnterCriticalSection(&criticalSection);

	_id = _data;

	clientSockList.push_back(*_sock);

	std::string message = _id + " : Enter new user in room.";

	LeaveCriticalSection(&criticalSection);

	if (!WriteAllPacket(_sock, message, false))
		return false;

	printf("[TCP/%s:%d] Login ID : %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), _id.c_str());

	return true;
}

BOOL ChatServer::PROC_PACKET_DATA(SOCKET* _sock, std::string& _id, char* _data)
{
	std::string message = _id + " : " + _data;

	if (!WriteAllPacket(_sock, message, false))
		return false;

	printf("[TCP/%s:%d] %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), message.c_str());

	return true;
}