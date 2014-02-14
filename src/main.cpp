//- Standard Library -
#include <iostream>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Thread.h>
#include <BeachJudge/Socket.h>

using namespace std;
using namespace beachjudge;

void *clientFunc(void *arg)
{
	Socket *client = Socket::Create(Socket::IP4, Socket::Stream, Socket::TCP);

	client->Connect("127.0.0.1", 1234);

	const char *msg = "Hello World!";
	print("Sending Msg: %s\n", msg);
	client->Write((char *)msg, strlen(msg));

	client->Shutdown();
	delete client;

	Thread::Exit(NULL);
	return 0;
}

void *serverFunc(void *arg)
{
	Socket *server = Socket::Create(Socket::IP4, Socket::Stream, Socket::TCP);

	server->Bind(1234);
	server->Listen(16);

	char sbuff[256];
	while(true)
	{
		Socket *client = server->Accept();

		if(client)
		{
			memset(sbuff, 0, 256);
			unsigned short len = client->Read(sbuff, 256);
			print("Receiving Msg: %d %s\n", len, sbuff);

			client->Shutdown();
			delete client;
		}

		sleepMS(1);
	}

	delete server;

	Thread::Exit(NULL);
	return 0;
}

int main(int argc, char **argv)
{
	print("Beach Judge v%s\n", getVersionString().c_str());

	Thread threadA(&serverFunc), threadB(&clientFunc);

	print("o-> Main Started\n");//
	threadA.Start((void *)"Thread A");
	sleepMS(1000);
	threadB.Start((void *)"Thread B");

	threadA.Join();
	threadB.Join();

	print("o-> Main Ended\n");
	Thread::Exit(NULL);
	return 0;
}
