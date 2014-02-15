//- Standard Library -
#include <cstdlib>
#include <map>
#include <queue>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>

//- Beach Judge -
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Base.h>

#define BEACHJUDGE_SESSION_EXPIREMS 30 * 60 * 1000 //- TODO: Externalize to Config -

using namespace std;
//\r\nSet-Cookie: BEACHJUDGESESSID=123456789
const char *header_OK = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/html\r\n\r\n";
const char *wwwPrefix = "../www/";

namespace beachjudge
{
	map<string, Page *> g_pageMap;
	map<string, void (*)(stringstream &, Socket *, Session *)> g_templateMap;

	void Page::RegisterTemplate(string entry, void (*func)(stringstream &, Socket *, Session *))
	{
		g_templateMap[entry] = func;
	}
	Page *Page::Create(string file)
	{
		if(!fileExists(file.c_str()))
			return 0;

		if(g_pageMap.count(file))
			return g_pageMap[file];

		string html, lineIn;
		ifstream inFile(file.c_str());
		while(getline(inFile, lineIn))
			html = html.append(lineIn);

		Page *page = new Page();
		page->m_fileSource = file;
		page->m_html = html;
		g_pageMap[file] = page;
		return page;
	}
	void Page::Cleanup()
	{
		while(g_pageMap.size())
			delete g_pageMap.begin()->second;
	}

	Page::Page()
	{
	}
	Page::~Page()
	{
		g_pageMap.erase(m_fileSource);
	}
	void Page::AddToStream(stringstream &stream, Socket *client, Session *session)
	{
		stringstream pageStream(m_html);
		string chunk, varChunk;
		while(getline(pageStream, chunk, '$'))
		{
			stream << chunk;

			if(pageStream.eof())
				break;

			varChunk = "";
			while(true)
			{
				char peek = pageStream.peek();
				if((peek >= 'A' && peek <= 'Z') || (peek >= 'a' && peek <= 'z'))
				{
					varChunk.push_back(peek);
					pageStream.get();
				}
				else
					break;
			}
			cout << "Smart Chunk: |" << varChunk << "|" << endl;
			if(g_templateMap.count(varChunk))
				g_templateMap[varChunk](stream, client, session);
			else
				stream << "$" << varChunk;
		}

		delete this;
	}

	bool SessionExpireComp(Session *sessA, Session *sessB)
	{
		return sessA->GetExpireTimeMS() > sessB->GetExpireTimeMS();
	}

	map<unsigned long, Session *> g_sessionMap;
	map<unsigned short, Session *> g_sessionIDMap;
	vector<Session *> g_sessionVec;

	Session *Session::Create(unsigned long address, unsigned short port, unsigned short userID)
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
		session->m_userID = userID;
		sort(g_sessionVec.begin(), g_sessionVec.end(), SessionExpireComp);
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
		}
	}

	Session::Session()
	{
		m_id = 0;
		m_userID = 0;
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
	unsigned short Session::GetUserID() const
	{
		return m_userID;
	}

	void HTTP::HandleRequest(Socket *client, std::string &request)
	{
		unsigned short port = 0;
		unsigned long addr = 0;
		client->GetPeerIP4Info(&addr, &port);

		unsigned char ip[4], *ipPtr = (unsigned char *)&addr;
		for(unsigned char a = 0; a < 4; a++)
		{
			ip[a] = *ipPtr & 0xFF;
			ipPtr++;
		}

		Session *session = Session::Create(addr, port, 5);
		stringstream stream(request);
		string method;
		stream >> method;
		print("[%d: %d %d] Receiving Msg: %d.%d.%d.%d:%d %d\r\n", getRunTimeMS(), session, session->GetID(), (unsigned short)ip[0], (unsigned short)ip[1], (unsigned short)ip[2], (unsigned short)ip[3], port);
		cout << request << endl;

		if(!method.compare("GET"))
		{
			string arguments;
			stream >> arguments;

			string in;
			while(stream >> in)
			{
				if(!in.compare("Cookie:"))
				{
					string cookie;
					while(getline(stream, cookie, '='))
					{
						string value;
						stream >> value;
						if(value.back() == ';')
							value.pop_back();

						if(!cookie.compare(" BEACHJUDGESESSID"))
						{
							print("Beach Judge Sess ID: %s\r\n", value.c_str());
						}
					}

					break;
				}
			}

			stringstream argStream(arguments);
			string arg, filePath = wwwPrefix;

			stringstream webPageStream;
			webPageStream << header_OK;

			getline(argStream, arg, '/');
			if(getline(argStream, arg, '/'))
			{
				string testPath = filePath;
				testPath.append(arg);
				testPath.append(".html");
				if(fileExists(testPath.c_str()))
					filePath = testPath;
				else
					filePath.append("404.html");
			}
			else
				filePath.append("index.html");

			Page *index = Page::Create(filePath);
			index->AddToStream(webPageStream, client, session);
			string webPage = webPageStream.str();
			client->Write((char *)webPage.c_str(), webPage.length());
		}
	}
	void HTTP::AppendHeader_OK(string &str)
	{
		str.append(header_OK);
	}
}
