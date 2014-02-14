//- Standard Library -
#include <iostream>
#include <string>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Thread.h>
#include <BeachJudge/Socket.h>

using namespace std;
using namespace beachjudge;

void *serverFunc(void *arg)
{
	Socket *server = Socket::Create(Socket::IP4, Socket::Stream, Socket::TCP);

	server->Bind(8081);
	server->Listen(16);

	char sbuff[8192];
	const char *writeStr = "Must test, wow.";
	while(true)
	{
		Socket *client = server->Accept();

		if(client)
		{
			unsigned short port = 0;
			unsigned long addr = 0;
			client->GetIP4Info(&addr, &port);

			memset(sbuff, 0, 8192);
			unsigned short len = client->Read(sbuff, 8191);
			print("Receiving Msg: %d.%d.%d.%d:%d %d\n", (addr & 0xFF000000 >> 24), (addr & 0x00FF0000 >> 16), (addr & 0x0000FF00 >> 8), (addr & 0x000000FF), port,  len);
			cout << sbuff << endl;
			
			client->Write((char *)writeStr, strlen(writeStr));

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

	Thread threadA(&serverFunc);

	print("o-> Main Started\n");//
	threadA.Start((void *)"Thread A");

	threadA.Join();

	print("o-> Main Ended\n");
	Thread::Exit(NULL);
	return 0;
}
