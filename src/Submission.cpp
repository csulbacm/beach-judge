//- Standard Library -
#include <map>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Submission.h>
#include <BeachJudge/Team.h>

#ifdef _WIN32
	#define SPRINTF sprintf_s
#else
	#define SPRINTF	sprintf
#endif

using namespace std;

#define TEAM_MAX_NUMACTIVESUBMISSIONS 10

namespace beachjudge
{
	map<unsigned short, Submission *> g_submissionsByID;

	Submission *Submission::Create(Team *team, Problem *problem, CodeType codeType, unsigned long timeMS, unsigned short id)
	{
		if(team->GetNumActiveSubmissions() >= TEAM_MAX_NUMACTIVESUBMISSIONS)
			return 0;

		Submission *submission = new Submission();

		if(id == 0)
		{
			do
			{
				submission->m_id = (unsigned short)rand();
			}
			while(g_submissionsByID.count(submission->m_id));
		}
		else
			submission->m_id = id; //- TODO: Clean this up -
		g_submissionsByID[submission->m_id] = submission;

		submission->m_codeType = codeType;
		submission->m_team = team;
		submission->m_problem = problem;
		submission->m_timeMS = timeMS;

		string sourceFile = "compo/submissions/";
		createFolder(sourceFile.c_str());
		char idStr[8];
		memset(idStr, 0, 8);
		SPRINTF(idStr, "%d", submission->m_id);
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
	map<unsigned short, Submission *> &Submission::GetSubmissionsByID()
	{
		return g_submissionsByID;
	}
	Submission *Submission::LookupByID(unsigned short id)
	{
		if(g_submissionsByID.count(id))
			return g_submissionsByID[id];
		return 0;
	}
	void Submission::Cleanup()
	{
		while(g_submissionsByID.size())
			delete g_submissionsByID.rbegin()->second;
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
		if(m_team)
			m_team->RemoveSubmission(this);
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
	CodeType Submission::GetCodeType() const
	{
		return m_codeType;
	}
	string Submission::GetSourceFile() const
	{
		return m_sourceFile;
	}
}