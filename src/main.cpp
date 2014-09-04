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

#define BEACHJUDGE_SESSION_CLEANUPTICKMS 15 * 60 * 1000 //TODO: Externalize to Config
#define BEACHJUDGE_COMPETITION_TICKMS 1000 //TODO: Externalize to Config

using namespace std;
using namespace beachjudge;

void *commandFunc(void *arg)
{
	string cmd;
	while(getline(cin, cmd))
	{
		if(!cmd.compare("help"))
		{
			print("Commands:\n");
			print("  sessions - displays active sessions\n");
			print("  start - starts the competition\n");
			print("  stop - stops the competition\n");
			print("  clear - clears all competition data\n");
		}
		else if(!cmd.compare("sessions"))
		{
			HTTP::LockActionMutex();
			Session::ListCurrent();
			HTTP::UnlockActionMutex();
		}
		else if(!cmd.compare("start"))
		{
			HTTP::LockActionMutex();
			Competition *competition = Competition::GetCurrent();
			if(competition)
			{
				competition->Start();
				competition->SaveToFile("compo/compo.txt");
			}
			HTTP::UnlockActionMutex();
		}
		else if(!cmd.compare("stop"))
		{
			HTTP::LockActionMutex();
			Competition *competition = Competition::GetCurrent();
			if(competition)
			{
				competition->Stop();
				competition->SaveToFile("compo/compo.txt");
			}
			HTTP::UnlockActionMutex();
		}
		else if(!cmd.compare("clear"))
		{
			HTTP::LockActionMutex();
			Competition *competition = Competition::GetCurrent();
			if(competition)
				competition->ClearAll();
			HTTP::UnlockActionMutex();
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
	delete data;

	HTTP::HandleClient(client);

	delete client;
	thread->End();
	Thread::Exit(0);
	return 0;
}

void *webServerFunc(void *arg)
{
	srand((unsigned int)time(0));

	//- Create server socket and listen -
	Socket *server = Socket::Create(Socket::IP4, Socket::Stream, Socket::TCP);
	server->Bind(8081);
	server->Listen(16);

	//- Connection handling process -
	while(true)
	{
		Socket *client = server->Accept();
		Thread *clientThread = new Thread(&clientHandlerFunc);

		pair<Socket *, Thread *> *data = new pair<Socket *, Thread *>();
		data->first = client;
		data->second = clientThread;
		clientThread->Start(data); //TODO: Confirm that this cleans up properly

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

	//------------------
	//- Initial Config -
	//------------------

	Team::Create("judge", "root", 0, true);
//	Team::Create("dummy", "1234", 1);

	Team::SetDatabase("compo/teams.txt");
	Team::LoadFromDatabase();

	Session::LoadFromFile("compo/sessions.txt");

	if(fileExists("compo/problems.txt"))
		Problem::LoadFromFile("compo/problems.txt");
	else
	{
		Problem::Create("The Skyline Problem");
		Problem::Create("Ugly Numbers");
	}

	Competition *competition = Competition::CreateFromFile("compo/compo.txt");
	if(!competition)
		competition = Competition::Create(7200000);
//		competition = Competition::Create(10800000); //7200000
	competition->SetCurrent();

	Team::LoadScores();

	Page::RegisterDefaultTemplates();

	//---------------------
	//- Launch Web Server -
	//---------------------

	Thread webServerThread(&webServerFunc);
	webServerThread.Start(0);

	//----------------------------
	//- Launch Command Interface -
	//----------------------------

	Thread commandThread(&commandFunc);
	commandThread.Start(0);

	//-------------------
	//- Session Timeout -
	//-------------------

	unsigned long long sessionCleanupMS = getRunTimeMS() + BEACHJUDGE_SESSION_CLEANUPTICKMS;
	unsigned long long competitionTickMS = getRunTimeMS() + BEACHJUDGE_COMPETITION_TICKMS;
	while(true)
	{
		unsigned long long currTimeMS = getRunTimeMS();

		if(currTimeMS >= competitionTickMS)
		{
			HTTP::LockActionMutex();
			if(competition->GetTimeLeft() == 0)
				competition->Stop();
			HTTP::UnlockActionMutex();
			competitionTickMS += BEACHJUDGE_COMPETITION_TICKMS;
		}

		if(currTimeMS >= sessionCleanupMS)
		{
			Session::Cleanup(); //TODO: Investigate stability
			sessionCleanupMS += BEACHJUDGE_SESSION_CLEANUPTICKMS;
		}

		Thread::DeleteDead();
		sleepMS(500);
	}

	//-----------
	//- Cleanup -
	//-----------

	commandThread.Join();
	webServerThread.Join();
	Question::Cleanup();
	Session::Cleanup(true);
	delete competition;

	Thread::Exit(NULL);
	return 0;
}
