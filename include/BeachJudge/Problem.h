#ifndef _BEACHJUDGE_PROBLEM_H_
#define _BEACHJUDGE_PROBLEM_H_

//- Standard Library -
#include <map>
#include <vector>
#include <string>

//- Beach Judge -
#include <BeachJudge/Submission.h>

namespace beachjudge
{
	class Problem
	{
		unsigned short m_id;
		std::string m_name, m_pdfPath;
		std::vector<std::pair<std::string, std::string> > m_tests;
		std::vector<Team *> m_solvers;

		Problem();

	public:
		static Problem *Create(std::string name);
		static Problem *LookupByID(unsigned short id);
		static std::map<unsigned short, Problem *> &GetProblemsByID();
		static void LoadFromFile(const char *file);
		static void SaveToFile(const char *file);
		static void ClearSolvers();

		~Problem();

		std::string GetName() const;
		void SetName(std::string name);
		unsigned short GetID() const;
		void SetID(unsigned short id);
		void AddSolver(Team *team);
		void RemoveSolver(Team *team);
		std::vector<Team *> *GetSolvers();
	};
}

#endif
