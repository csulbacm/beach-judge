#ifndef _BEACHJUDGE_PROBLEM_H_
#define _BEACHJUDGE_PROBLEM_H_

//- Standard Library -
#include <map>
#include <vector>
#include <string>

namespace beachjudge
{
	class Team;
	class Problem;

	class Attempt
	{
		Team *m_team;
		Problem *m_problem;
		unsigned long m_timeMS;

	public:
		Attempt(Team *team, Problem *problem, unsigned long timeMS);

		Team *GetTeam() const;
		Problem *GetProblem() const;
		unsigned long GetTimeMS() const;
	};

	class Solution : public Attempt
	{
		unsigned short m_penalties;

	public:
		Solution(Team *team, Problem *problem, unsigned long timeMS, unsigned short penalties);
	};

	class Problem
	{
		unsigned short m_id;
		std::string m_name, m_pdfPath;
		std::map<unsigned long, Solution *> m_solutions;
		std::vector<std::pair<std::string, std::string> > m_tests;

		Problem();

	public:
		static Problem *Create(unsigned short id, std::string name);
		static std::map<unsigned short, Problem *> &GetProblemsByID();

		~Problem();

		std::string GetName() const;
		unsigned short GetID() const;
	};
}

#endif
