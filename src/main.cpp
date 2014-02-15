//- Standard Library -
#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Thread.h>
#include <BeachJudge/Socket.h>

#define BEACHJUDGE_SESSION_CLEANUPTICKMS 15 * 60 * 1000 //- TODO: Externalize to Config -

using namespace std;
using namespace beachjudge;

void *webServerFunc(void *arg)
{
	srand((unsigned int)time(0));
	Socket *server = Socket::Create(Socket::IP4, Socket::Stream, Socket::TCP);

	server->Bind(8081);
	server->Listen(16);

	char sbuff[8192];
	while(true)
	{
		Socket *client = server->Accept(); //- TODO: Fix Non-Blocking Sockets -

		if(client)
		{
			memset(sbuff, 0, 8192);
			unsigned short len = client->Read(sbuff, 8191);

			string request(sbuff);
			HTTP::HandleRequest(client, request);

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
	srand((unsigned int)time(0));

	print("Beach Judge v%s\n", getVersionString().c_str());

	Thread webServerThread(&webServerFunc);

	webServerThread.Start(0);

	unsigned long sessionCleanupMS = getRunTimeMS() + BEACHJUDGE_SESSION_CLEANUPTICKMS;
	while(true)
	{
		unsigned long currTimeMS = getRunTimeMS();

		if(currTimeMS >= sessionCleanupMS)
		{
			Session::Cleanup(); //- TODO: Investigate stability -
			sessionCleanupMS += BEACHJUDGE_SESSION_CLEANUPTICKMS;
		}

		sleepMS(5000);
	}

	webServerThread.Join();
	Session::Cleanup(true);
	Thread::Exit(NULL);
	return 0;
}
