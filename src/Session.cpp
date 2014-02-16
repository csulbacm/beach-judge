//- Standard Library -
#include <map>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Session.h>

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

	Session *Session::Create(unsigned long address, unsigned short port, Team *team)
	{
		Session *session = Lookup(address);
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
				sprintf(buff, "%d", (unsigned short)(address & 0xFF));
				addrStr.append(buff);
				address >>= 8;
				if(a < 3)
					addrStr.push_back('.');
			}
			session->m_addressString = addrStr;

			do
			{
				session->m_id = (unsigned short)rand();
			}
			while(g_sessionIDMap.count(session->m_id));
			g_sessionIDMap[session->m_id] = session;
			g_sessionVec.push_back(session);
		}

		session->m_port = port;
		session->m_team = team;
		session->ResetTimeout();
		return session;
	}
	Session *Session::Lookup(unsigned long address)
	{
		if(g_sessionMap.count(address))
			return g_sessionMap[address];
		return 0;
	}
	void Session::Cleanup(bool deleteAll)
	{
		unsigned long currTimeMS = getRunTimeMS();
		while(g_sessionVec.size())
		{
			Session *session = g_sessionVec.back();
			if(deleteAll || session->GetExpireTimeMS() <= currTimeMS)
				delete session;
			else
				break;
		}
	}

	void Session::ListCurrent()
	{
		if(g_sessionMap.size())
		{
			cout << "Current Sessions:" << endl;
			unsigned long currTimeMS = getRunTimeMS();
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
	unsigned long Session::GetExpireTimeMS() const
	{
		return m_expireTimeMS;
	}
	unsigned short Session::GetID() const
	{
		return m_id;
	}
	Team *Session::GetTeam() const
	{
		return m_team;
	}
	unsigned short Session::GetVariable(std::string name)
	{
		if(m_variables.count(name))
			return m_variables[name];
		return 0;
	}
	void Session::ResetTimeout()
	{
		m_expireTimeMS = getRunTimeMS() + BEACHJUDGE_SESSION_EXPIREMS;
		sort(g_sessionVec.begin(), g_sessionVec.end(), SessionExpireComp);
	}
}
