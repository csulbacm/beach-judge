//- Standard Library -
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Thread.h>
#include <BeachJudge/Socket.h>

using namespace std;
using namespace beachjudge;

const char *helloWorld = "<html>\r\n<head>\r\n<title>Hello, world!</title>\r\n</head>\r\n<body>\r\n<h1>Hello, world!</h1>\r\n</body>\r\n</html>\r\n\r\n";

void *serverFunc(void *arg)
{
	Socket *server = Socket::Create(Socket::IP4, Socket::Stream, Socket::TCP);

	server->Bind(8081);
	server->Listen(16);

	char sbuff[8192];
	string page = "";
	HTTP::AppendHeader_OK(page);
	page.append(helloWorld);
	while(true)
	{
		Socket *client = server->Accept();

		if(client)
		{
			unsigned short port = 0;
			unsigned long addr = 0;
			client->GetPeerIP4Info(&addr, &port);

			memset(sbuff, 0, 8192);
			unsigned short len = client->Read(sbuff, 8191);
			unsigned char ip[4], *ipPtr = (unsigned char *)&addr;
			for(unsigned char a = 0; a < 4; a++)
			{
				ip[a] = *ipPtr & 0xFF;
				ipPtr++;
			}

			stringstream stream(sbuff);
			string method;
			stream >> method;
			print("Receiving Msg: %d.%d.%d.%d:%d %d\r\n", (unsigned short)ip[0], (unsigned short)ip[1], (unsigned short)ip[2], (unsigned short)ip[3], port,  len);
			cout << sbuff << endl;

			if(!method.compare("GET"))
			{
				string file;
				stream >> file;
//				print("Accessing File: %s\r\n", file.c_str());
				client->Write((char *)page.c_str(), page.length());
			}

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
