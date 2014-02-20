//- Standard Library -
#include <map>
#include <cstdlib>
#include <cstring>
#include <cstdio>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Problem.h>
#include <BeachJudge/Team.h>

using namespace std;

#define TEAM_MAX_NUMACTIVESUBMISSIONS 10

namespace beachjudge
{
	map<unsigned short, Problem *> g_problemsByID;
	map<unsigned short, Submission *> g_submissionsByID;

	Submission *Submission::Create(Team *team, Problem *problem, CodeType codeType, unsigned long timeMS)
	{
		if(team->GetNumActiveSubmissions() >= TEAM_MAX_NUMACTIVESUBMISSIONS)
			return 0;

		Submission *submission = new Submission();
		do
		{
			submission->m_id = (unsigned short)rand();
		}
		while(g_submissionsByID.count(submission->m_id));
		g_submissionsByID[submission->m_id] = submission;

		submission->m_codeType = codeType;
		submission->m_team = team;
		submission->m_problem = problem;
		submission->m_timeMS = timeMS;

		string sourceFile = "submissions/";
		createFolder(sourceFile.c_str());
		char idStr[8];
		memset(idStr, 0, 8);
		sprintf(idStr, "%d", submission->m_id);
		sourceFile.append(string(idStr));
		switch(codeType)
		{
		case CodeType_CPP:
			sourceFile.append(".cpp");
			break;
		case CodeType_C:
			sourceFile.append(".c");
			break;
		case CodeType_Java:
			sourceFile.append(".java");
			break;
		case CodeType_Unknown:
			break;
		}
		submission->m_sourceFile = sourceFile;
		team->AddSubmission(submission);
		return submission;
	}

	Submission::Submission()
	{
		m_id = 0;
		m_timeMS = 0;
		m_team = 0;
		m_problem = 0;
		m_codeType = CodeType_Unknown;
	}
	Submission::~Submission()
	{
		g_submissionsByID.erase(m_id);
	}
	Team *Submission::GetTeam() const
	{
		return m_team;
	}
	Problem *Submission::GetProblem() const
	{
		return m_problem;
	}
	unsigned long Submission::GetTimeMS() const
	{
		return m_timeMS;
	}
	unsigned short Submission::GetID() const
	{
		return m_id;
	}
	string Submission::GetSourceFile() const
	{
		return m_sourceFile;
	}

	Problem *Problem::Create(unsigned short id, string name)
	{
		if(g_problemsByID.count(id))
			return g_problemsByID[id];
		Problem *problem = new Problem();
		g_problemsByID[id] = problem;
		problem->m_id = id;
		problem->m_name = name;

		return problem;
	}
	Problem *Problem::LookupByID(unsigned short id)
	{
		if(g_problemsByID.count(id))
			return g_problemsByID[id];
		return 0;
	}
	map<unsigned short, Problem *> &Problem::GetProblemsByID()
	{
		return g_problemsByID;
	}

	Problem::Problem()
	{
		m_id = 0;
	}
	Problem::~Problem()
	{
		g_problemsByID.erase(m_id);
	}
	string Problem::GetName() const
	{
		return m_name;
	}
	unsigned short Problem::GetID() const
	{
		return m_id;
	}
}
