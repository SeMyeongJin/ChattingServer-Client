#include "ChatServer.h"

int main(int argc, char *argv[])
{
	ChatServer server;

	if (!server.Socket())
		return 0;

	if (!server.Bind(SERVERPORT))
		return 0;

	if (!server.Listen(SOMAXCONN))
		return 0;

	server.AcceptLoop();

	return 0;
}