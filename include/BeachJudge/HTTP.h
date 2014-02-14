#ifndef _BEACHJUDGE_HTTP_H_
#define _BEACHJUDGE_HTTP_H_

//- Standard Library -
#include <string>

using namespace std;

namespace beachjudge
{

	class Session
	{
		unsigned long m_address, m_expireTimeMS;
		unsigned short m_id, m_port;

		Session();

	public:
		static void Cleanup(bool deleteAll = false);
		static Session *Create(unsigned long address, unsigned short port);

		unsigned short GetID() const;
		unsigned long GetExpireTimeMS() const;

		~Session();
	};

	class HTTP
	{
	public:
		static void AppendHeader_OK(string &str);
	};
}

#endif
