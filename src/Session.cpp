//- Standard Library -
#include <map>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Session.h>

#ifdef _WIN32
	#define SPRINTF sprintf_s
#else
	#define SPRINTF	sprintf
#endif

using namespace std;

#define BEACHJUDGE_SESSION_EXPIREMS 30 * 60 * 1000 //- TODO: Externalize to Config -

namespace beachjudge
{
	bool SessionExpireComp(Session *sessA, Session *sessB)
	{
		return sessA->GetExpireTimeMS() > sessB->GetExpireTimeMS();
	}

	map<unsigned long, Session *> g_sessionMap;
	map<unsigned short, Session *> g_sessionIDMap;
	vector<Session *> g_sessionVec;

	Session *Session::Create(unsigned long address, unsigned short port, Team *team, unsigned short id)
	{
		Session *session = Lookup(id);
		if(!session)
		{
			session = new Session();
			session->m_address = address;
			string addrStr;
			g_sessionMap[address] = session;
			for(unsigned char a = 0; a < 4; a++)
			{
				char buff[4];
				memset(buff, 0, 4);
				SPRINTF(buff, "%d", (unsigned short)(address & 0xFF));
				addrStr.append(buff);
				address >>= 8;
				if(a < 3)
					addrStr.push_back('.');
			}
			session->m_addressString = addrStr;

			//- TODO: Make sure this is safe -
			if(id)
				session->m_id = id;
			else
			{
				do
				{
					session->m_id = (unsigned short)rand();
				}
				while(g_sessionIDMap.count(session->m_id) || session->m_id == 0);
			}
			g_sessionIDMap[session->m_id] = session;
			g_sessionVec.push_back(session);
		}

		session->m_variables["isJudge"] = team->IsJudge() ? 1 : 0;
		session->m_variables["isTeam"] = team->IsJudge() ? 0 : 1;
		session->m_port = port;
		session->m_team = team;
		if(id == 0) //- TODO: This should be cleaner, but this occurs if the session was loaded from the file -
			session->ResetTimeout();
		return session;
	}
	Session *Session::Lookup(unsigned short id)
	{
		if(g_sessionIDMap.count(id))
			return g_sessionIDMap[id];
		return 0;
	}
	Session *Session::LookupByAddress(unsigned long address)
	{
		if(g_sessionMap.count(address))
			return g_sessionMap[address];
		return 0;
	}
	void Session::Cleanup(bool deleteAll)
	{
		unsigned long long currTimeMS = getRealTimeMS();
		while(g_sessionVec.size())
		{
			Session *session = g_sessionVec.back();
			if(deleteAll || session->GetExpireTimeMS() <= currTimeMS)
				delete session;
			else
				break;
		}
		if(!deleteAll)
			Session::SaveAll();
	}
	void Session::SaveAll() //- TODO: Don't save all sessions in one file -
	{
		createFolder("compo");
		ofstream outFile("compo/sessions.txt");
		for(map<unsigned short, Session *>::iterator it = g_sessionIDMap.begin(); it != g_sessionIDMap.end(); it++)
		{
			Session *session = it->second;
			outFile << session->m_id << "\t" << session->m_address << "\t" << session->m_port << "\t" << session->m_team->GetID() << "\t" << session->m_expireTimeMS << endl;
		}
		outFile.close();
	}
	void Session::LoadFromFile(const char *file)
	{
		if(fileExists(file))
		{
			ifstream inFile(file);
			string in;
			while(getline(inFile, in))
			{
				stringstream inStream(in);
				unsigned short id, port, teamID;
				unsigned long address;
				unsigned long long expireTimeMS;
				inStream >> id >> address >> port >> teamID >> expireTimeMS;

				Team *team = Team::LookupByID(teamID);
				if(team)
					Session::Create(address, port, team, id);
			}
		}
	}
	void Session::ListCurrent()
	{
		if(g_sessionMap.size())
		{
			cout << "Current Sessions:" << endl;
			unsigned long long currTimeMS = getRealTimeMS();
			for(map<unsigned long, Session *>::iterator it = g_sessionMap.begin(); it != g_sessionMap.end(); it++)
			{
				Session *session = it->second;
				cout << "+ " << session->m_id << '\t' << session->m_addressString << ":" << session->m_port << '\t' << session->m_team->GetName() << '\t' << (session->m_expireTimeMS - currTimeMS) << endl;
			}
		}
		else
			cout << "No Current Sessions" << endl;
	}

	Session::Session()
	{
		m_id = 0;
		m_team = 0;
		m_expireTimeMS = 0;
		m_variables["loggedIn"] = 1;
	}
	Session::~Session()
	{
		g_sessionMap.erase(this->m_address);
		g_sessionIDMap.erase(this->m_id);
		g_sessionVec.erase(find(g_sessionVec.begin(), g_sessionVec.end(), this));
	}
	unsigned long long Session::GetExpireTimeMS() const
	{
		return m_expireTimeMS;
	}
	unsigned short Session::GetID() const
	{
		return m_id;
	}
	unsigned long Session::GetAddress() const
	{
		return m_address;
	}
	Team *Session::GetTeam() const
	{
		return m_team;
	}
	unsigned short Session::GetVariable(string name)
	{
		if(m_variables.count(name))
			return m_variables[name];
		return 0;
	}
	void Session::SetVariable(string name, unsigned short val)
	{
		m_variables[name] = val;
	}
	void Session::ClearVariable(string name)
	{
		m_variables.erase(name);
	}
	void Session::ResetTimeout()
	{
		m_expireTimeMS = getRealTimeMS() + BEACHJUDGE_SESSION_EXPIREMS;
		sort(g_sessionVec.begin(), g_sessionVec.end(), SessionExpireComp);
		Session::SaveAll();
	}
}
