#ifndef _BEACHJUDGE_SESSION_H_
#define _BEACHJUDGE_SESSION_H_

//- Standard Library -
#include <string>
#include <map>

namespace beachjudge
{
	class Session
	{
		unsigned long m_address, m_expireTimeMS;
		unsigned short m_id, m_port, m_userID;
		std::map<std::string, unsigned short> m_variables;

		Session();

	public:
		static void Cleanup(bool deleteAll = false);
		static Session *Create(unsigned long address, unsigned short port, unsigned short userID);
		static Session *Lookup(unsigned long address);

		unsigned short GetID() const;
		unsigned short GetUserID() const;
		unsigned long GetExpireTimeMS() const;
		unsigned short GetVariable(std::string name);

		~Session();
	};
}

#endif
