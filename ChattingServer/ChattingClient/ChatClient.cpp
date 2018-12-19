#include "ChatClient.h"



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

ChatClient::ChatClient()
{
}

ChatClient::~ChatClient()
{
	WSACleanup();
}

BOOL ChatClient::Socket()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return FALSE;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) printf("Socket() Error!\n");

	return TRUE;
}

bool ChatClient::Connect(std::string _ip, USHORT _port)
{
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(_ip.c_str());
	serveraddr.sin_port = htons(_port);

	int retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR)
	{
		printf("connect() Error!\n");
		return FALSE;
	}

	return TRUE;
}

VOID ChatClient::ReceiveLoop()
{
	int retval = 0;
	char buf[BUFSIZE + 1];

	while (TRUE)
	{
		retval = recv(sock, buf, BUFSIZE, 0);

		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			printf("recv() Error!\n");
			return;
		}
		else if (retval == 0)
			break;

		buf[retval] = '\0';

		printf("%s \n", buf);
	}
}

VOID ChatClient::Loop()
{
	workThread = new std::thread(&ChatClient::ReceiveLoop, this);

	char buf[BUFSIZE];
	int len;

	printf("Entered chatting room!\n");
	printf("Set your ID : ");
	char str[BUFSIZE] = "";
	scanf("%s", str);

	PACKET_TYPE type = PACKET_TYPE::TYPE_LOGIN;
	PackingPacket(buf, &type, str, &len);

	int retval = send(sock, buf, len, 0);

	if (retval == SOCKET_ERROR) {
		printf("send() Error!\n");
		return;
	}

	while (true)
	{
		strcpy(str, "");
		scanf("%s", str);

		if (!strcmp(str, "/bye"))
		{
			return;
		}

		PACKET_TYPE type = PACKET_TYPE::TYPE_DATA;
		PackingPacket(buf, &type, str, &len);

		retval = send(sock, buf, len, 0);

		if (retval == SOCKET_ERROR)
		{
			printf("send() Error!\n");
			break;
		}
	}

	closesocket(sock);
}