#ifndef _BEACHJUDGE_TEAM_H_
#define _BEACHJUDGE_TEAM_H_

//- Standard Library -
#include <string>
#include <cstdlib>

//- Beach Judge -
#include <BeachJudge/Socket.h>

namespace beachjudge
{
	class Team
	{
		unsigned short m_id;
		std::string m_name, m_password;

		Team();

	public:
		static Team *Create(std::string name, std::string password, unsigned short id = -1);
		static Team *LookupByID(unsigned short id);
		static Team *LookupByName(std::string name);
		static void SetDatabase(std::string file);
		static void SaveToDatabase();
		static void LoadFromDatabase();
		static void DeleteAll();

		~Team();

		bool TestPassword(std::string password);
		std::string GetName() const;
	};
}

#endif
