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
		unsigned short m_id, m_totalPenalties;
		unsigned long m_totalScore;
		std::string m_name, m_password;
		bool m_isJudge;

		Team();

	public:
		static Team *Create(std::string name, std::string password, unsigned short id = 0, bool isJudge = false);
		static Team *LookupByID(unsigned short id);
		static Team *LookupByName(std::string name);
		static void SetDatabase(std::string file);
		static void SaveToDatabase();
		static void LoadFromDatabase();
		static void DeleteAll();

		~Team();

		void SetPassword(std::string password);
		bool TestPassword(std::string password);
		std::string GetName() const;
		unsigned short GetID() const;
		unsigned short GetTotalPenalties() const;
		unsigned long GetTotalScore() const;
		bool IsJudge() const;
	};
}

#endif
