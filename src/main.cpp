//- Standard Library -
#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Page.h>
#include <BeachJudge/Competition.h>
#include <BeachJudge/Problem.h>
#include <BeachJudge/Thread.h>
#include <BeachJudge/Socket.h>
#include <BeachJudge/Question.h>
#include <BeachJudge/Team.h>

#define BEACHJUDGE_SESSION_CLEANUPTICKMS 15 * 60 * 1000 //- TODO: Externalize to Config -

using namespace std;
using namespace beachjudge;

void *commandFunc(void *arg)
{
	string cmd;
	while(getline(cin, cmd))
	{
		if(!cmd.compare("sessions"))
			Session::ListCurrent();
		if(!cmd.compare("start"))
		{
			Competition *competition = Competition::GetCurrent();
			if(competition)
			{
				competition->Start();
				competition->SaveToFile("compo/compo.txt");
			}
		}
	}

	Thread::Exit(0);
	return 0;
}

void *clientHandlerFunc(void *arg)
{
	srand((unsigned int)time(0));
	pair<Socket *, Thread *> *data = (pair<Socket *, Thread *> *)arg;
	Thread *thread = data->second;
	Socket *client = data->first;

	HTTP::HandleClient(client);

	client->Shutdown();
	
	delete data;
//	delete thread;

	Thread::Exit(0);
	return 0;
}

void *webServerFunc(void *arg)
{
	srand((unsigned int)time(0));
	Socket *server = Socket::Create(Socket::IP4, Socket::Stream, Socket::TCP);

	server->Bind(8081);
	server->Listen(16);

	while(true)
	{
		Socket *client = server->Accept(); //- TODO: Fix Non-Blocking Sockets -

		Thread *clientThread = new Thread(&clientHandlerFunc);

		pair<Socket *, Thread *> *data = new pair<Socket *, Thread *>();
		data->first = client;
		data->second = clientThread;
		clientThread->Start(data); //- TODO: Confirm that this cleans up properly -

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

	Team::Create("judge", "root", 0, true);
	Team::Create("dummy", "1234", 1);

	Team::SetDatabase("compo/teams.txt");
	Team::LoadFromDatabase();

	Problem::Create(1, "The Skyline Problem");
	Problem::Create(2, "Ugly Numbers");

	Competition *competition = Competition::CreateFromFile("compo/compo.txt");
	if(!competition)
		competition = Competition::Create(7200000);
	competition->SetCurrent();

	Page::RegisterDefaultTemplates();

	Thread webServerThread(&webServerFunc);
	webServerThread.Start(0);

	Thread commandThread(&commandFunc);
	commandThread.Start(0);

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

	delete competition;
	commandThread.Join();
	webServerThread.Join();
	Question::Cleanup();
	Session::Cleanup(true);
	Thread::Exit(NULL);
	return 0;
}
