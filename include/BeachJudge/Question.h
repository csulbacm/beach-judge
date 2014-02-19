#ifndef _BEACHJUDGE_QUESTION_H_
#define _BEACHJUDGE_QUESTION_H_

//- Standard Library -
#include <string>
#include <vector>
#include <map>

//- Beach Judge -
#include <BeachJudge/Problem.h>
#include <BeachJudge/Team.h>

namespace beachjudge
{
	class Question
	{
		bool m_isAnswered;
		unsigned short m_id;
		std::string m_text, m_response;
		Team *m_team;
		Problem *m_problem;

		Question();

	public:
		static Question *Create(std::string text, Team *team, Problem *problem);
		static std::map<Team *, std::vector<Question *> > &GetQuestionsByTeam();
		static std::map<Problem *, std::vector<Question *> > &GetQuestionsByProblem();
		static Question *LookupByID(unsigned short id);
		static void Cleanup();

		~Question();

		void Answer(std::string response);
		bool IsAnswered() const;
		std::string GetText() const;
		std::string GetAnswer() const;
		Team *GetTeam() const;
		Problem *GetProblem() const;
		unsigned short GetID() const;
	};
}

#endif
