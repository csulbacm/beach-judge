#ifndef _BEACHJUDGE_PROBLEM_H_
#define _BEACHJUDGE_PROBLEM_H_

//- Standard Library -
#include <map>
#include <vector>
#include <string>

namespace beachjudge
{
	typedef enum CodeType
	{
		CodeType_Unknown,
		CodeType_C,
		CodeType_CPP,
		CodeType_Java
	} CodeType;

	class Team;
	class Problem;

	class Submission
	{
		CodeType m_codeType;
		unsigned short m_id;
		Team *m_team;
		Problem *m_problem;
		unsigned long m_timeMS;
		std::string m_sourceFile;

		Submission();

	public:
		static Submission *Create(Team *team, Problem *problem, CodeType codeType, unsigned long timeMS);

		~Submission();

		Team *GetTeam() const;
		Problem *GetProblem() const;
		unsigned long GetTimeMS() const;
		unsigned short GetID() const;
		std::string GetSourceFile() const;
	};

	class Solution : public Submission
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
		static Problem *LookupByID(unsigned short id);
		static std::map<unsigned short, Problem *> &GetProblemsByID();

		~Problem();

		std::string GetName() const;
		unsigned short GetID() const;
	};
}

#endif
