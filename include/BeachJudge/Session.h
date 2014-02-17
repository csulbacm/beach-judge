#ifndef _BEACHJUDGE_SESSION_H_
#define _BEACHJUDGE_SESSION_H_

//- Standard Library -
#include <string>
#include <map>

//- Beach Judge -
#include <BeachJudge/Team.h>

namespace beachjudge
{
	class Session
	{
		unsigned long m_address, m_expireTimeMS;
		unsigned short m_id, m_port;
		Team *m_team;
		std::string m_addressString;
		std::map<std::string, unsigned short> m_variables;

		Session();

	public:
		static void Cleanup(bool deleteAll = false);
		static Session *Create(unsigned long address, unsigned short port, Team *team);
		static Session *Lookup(unsigned short id);

		static void ListCurrent();

		unsigned short GetID() const;
		Team *GetTeam() const;
		unsigned long GetExpireTimeMS() const;
		unsigned long GetAddress() const;
		unsigned short GetVariable(std::string name);
		void ResetTimeout();

		~Session();
	};
}

#endif
