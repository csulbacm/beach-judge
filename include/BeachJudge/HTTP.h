#ifndef _BEACHJUDGE_HTTP_H_
#define _BEACHJUDGE_HTTP_H_

//- Standard Library -
#include <string>
#include <sstream>

//- Beach Judge -
#include <BeachJudge/Socket.h>

using namespace std;

namespace beachjudge
{
	class Session
	{
		unsigned long m_address, m_expireTimeMS;
		unsigned short m_id, m_port, m_userID;

		Session();

	public:
		static void Cleanup(bool deleteAll = false);
		static Session *Create(unsigned long address, unsigned short port, unsigned short userID);
		static Session *Lookup(unsigned long address);

		unsigned short GetID() const;
		unsigned short GetUserID() const;
		unsigned long GetExpireTimeMS() const;

		~Session();
	};

	class Page
	{
		string m_fileSource, m_html;

		Page();

	public:
		static Page *Create(string file);
		static void Cleanup();

		~Page();

		void AddToStream(stringstream &stream, Socket *client, Session *session);
	};

	class HTTP
	{
	public:
		static void AppendHeader_OK(string &str);
		static void HandleRequest(Socket *client, std::string &request);
	};
}

#endif
