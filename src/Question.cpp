//- Standard Library -
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Question.h>

using namespace std;

namespace beachjudge
{
	vector<Question *> g_questionsVec;
	map<Problem *, vector<Question *> > g_questionsByProblem;
	map<Team *, vector<Question *> > g_questionsByTeam;
	
	Question *Question::Create(string text, Team *team, Problem *problem)
	{
		Question *question = new Question();
		question->m_text = text;
		question->m_team = team;
		question->m_problem = problem;
		g_questionsByProblem[problem].push_back(question);
		g_questionsByTeam[team].push_back(question);
		g_questionsVec.push_back(question);
		return question;
	}
	void Question::Cleanup()
	{
		while(g_questionsVec.size())
			delete g_questionsVec.back();
	}
	map<Team *, vector<Question *> > &Question::GetQuestionsByTeam()
	{
		return g_questionsByTeam;
	}
	map<Problem *, vector<Question *> > &Question::GetQuestionsByProblem()
	{
		return g_questionsByProblem;
	}

	Question::Question()
	{
		m_isAnswered = false;
	}
	Question::~Question()
	{
		vector<Question *> &questionsByProblemVec = g_questionsByProblem[m_problem];
		questionsByProblemVec.erase(find(questionsByProblemVec.begin(), questionsByProblemVec.end(), this));

		vector<Question *> &questionsByTeamVec = g_questionsByTeam[m_team];
		questionsByTeamVec.erase(find(questionsByTeamVec.begin(), questionsByTeamVec.end(), this));

		g_questionsVec.erase(find(g_questionsVec.begin(), g_questionsVec.end(), this));
	}
	void Question::Answer(string response)
	{
		m_response = response;
		m_isAnswered = true;
	}
	bool Question::IsAnswered() const
	{
		return m_isAnswered;
	}
	string Question::GetText() const
	{
		return m_text;
	}
	string Question::GetAnswer() const
	{
		return m_response;
	}
	Team *Question::GetTeam() const
	{
		return m_team;
	}
	Problem *Question::GetProblem() const
	{
		return m_problem;
	}
}
