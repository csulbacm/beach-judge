//- Standard Library -
#include <cstdlib>
#include <map>
#include <queue>
#include <algorithm>

//- Beach Judge -
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Base.h>

#define BEACHJUDGE_SESSION_EXPIREMS 30 * 60 * 1000 //- TODO: Externalize to Config -

using namespace std;
//\r\nSet-Cookie: BEACHJUDGESESSID=123456789
const char *header_OK = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/html\r\n\r\n";

namespace beachjudge
{
	bool SessionExpireComp(Session *sessA, Session *sessB)
	{
		return sessA->GetExpireTimeMS() > sessB->GetExpireTimeMS();
	}

	map<unsigned long, Session *> g_sessionMap;
	map<unsigned short, Session *> g_sessionIDMap;
	vector<Session *> g_sessionVec;

	Session *Session::Create(unsigned long address, unsigned short port)
	{
		Session *session = 0;
		if(g_sessionMap.count(address))
		{
			session = g_sessionMap[address];
		}
		else
		{
			session = new Session();
			session->m_address = address;
			g_sessionMap[address] = session;

			do
			{
				session->m_id = (unsigned short)rand();
			}
			while(g_sessionIDMap.count(session->m_id));
			g_sessionIDMap[session->m_id] = session;
			g_sessionVec.push_back(session);
		}

		session->m_expireTimeMS = getRunTimeMS() + BEACHJUDGE_SESSION_EXPIREMS;
		session->m_port = port;
		sort(g_sessionVec.begin(), g_sessionVec.end(), SessionExpireComp);
		return session;
	}
	void Session::Cleanup(bool deleteAll)
	{
		unsigned long currTimeMS = getRunTimeMS();
		while(g_sessionVec.size())
		{
			Session *session = g_sessionVec.back();
			if(deleteAll || session->GetExpireTimeMS() <= currTimeMS)
				delete session;
		}
	}

	Session::Session()
	{
		m_id = 0;
		m_expireTimeMS = 0;
	}
	Session::~Session()
	{
		g_sessionVec.erase(find(g_sessionVec.begin(), g_sessionVec.end(), this));
		g_sessionMap.erase(this->m_address);
		g_sessionIDMap.erase(this->m_id);
	}
	unsigned long Session::GetExpireTimeMS() const
	{
		return m_expireTimeMS;
	}
	unsigned short Session::GetID() const
	{
		return m_id;
	}

	void HTTP::AppendHeader_OK(string &str)
	{
		str.append(header_OK);
	}
}
