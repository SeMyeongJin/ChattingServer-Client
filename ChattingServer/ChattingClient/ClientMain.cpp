#include "ChatClient.h"

int main(int argc, char *argv[])
{
	ChatClient client;

	if (!client.Socket())
		return 0;

	if (!client.Connect("127.0.0.1", SERVERPORT))
		return 0;

	client.Loop();

	return 0;
}