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
		std::map<unsigned long, Submission *> m_solutions;
		std::vector<std::pair<std::string, std::string> > m_tests;

		Problem();

	public:
		static Problem *Create(unsigned short id, std::string name);
		static Problem *LookupByID(unsigned short id);
		static std::map<unsigned short, Problem *> &GetProblemsByID();
		static void LoadFromFile(const char *file);

		~Problem();

		std::string GetName() const;
		unsigned short GetID() const;
	};
}

#endif
