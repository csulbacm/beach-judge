//- Standard Library -
#include <map>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <csignal>
#include <iostream>
#include <algorithm>

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
	vector<Submission *> g_pendingSubmissions;
	
	bool PendingSubmissionComp(Submission *subA, Submission *subB)
	{
		return subA->GetTimeMS() < subB->GetTimeMS();
	}

	Submission *Submission::Create(Team *team, Problem *problem, CodeType codeType, unsigned long long timeMS, SubStatus subStatus, unsigned short id)
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
		submission->m_subStatus = subStatus;
		submission->m_team = team;
		submission->m_problem = problem;
		submission->m_timeMS = timeMS;

		string sourceFile = "compo/submissions/";
		createFolder(sourceFile.c_str());
		char idStr[8];
		memset(idStr, 0, 8);
		SPRINTF(idStr, "%d", submission->m_id);
		sourceFile.append(string(idStr));
		submission->m_base = sourceFile;
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
		if(subStatus == SubStatus_Pending)
		{
			g_pendingSubmissions.push_back(submission);
			sort(g_pendingSubmissions.begin(), g_pendingSubmissions.end(), PendingSubmissionComp);
		}
		return submission;
	}
	map<unsigned short, Submission *> &Submission::GetSubmissionsByID()
	{
		return g_submissionsByID;
	}
	vector<Submission *> *Submission::GetPendingSubmissions()
	{
		return &g_pendingSubmissions;
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
		m_subStatus = SubStatus_Pending;
	}
	Submission::~Submission()
	{
		g_submissionsByID.erase(m_id);
		if(m_subStatus == SubStatus_Pending)
			g_pendingSubmissions.erase(find(g_pendingSubmissions.begin(), g_pendingSubmissions.end(), this));
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
	unsigned long long Submission::GetTimeMS() const
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
	string Submission::GetCodeTypeText() const
	{
		switch(m_codeType)
		{
		case CodeType_Unknown:
			return "Unknown";
		case CodeType_C:
			return "C";
		case CodeType_CPP:
			return "C++";
		case CodeType_Java:
			return "Java";
		}
		return "";
	}
	string Submission::GetSourceFile() const
	{
		return m_sourceFile;
	}
	SubStatus Submission::GetStatus() const
	{
		return m_subStatus;
	}
	void Submission::SetStatus(SubStatus status)
	{
		if(status == SubStatus_Pending && m_subStatus != SubStatus_Pending)
		{
			g_pendingSubmissions.push_back(this);
			sort(g_pendingSubmissions.begin(), g_pendingSubmissions.end(), PendingSubmissionComp);
		}
		if(status != SubStatus_Pending && m_subStatus == SubStatus_Pending)
			g_pendingSubmissions.erase(find(g_pendingSubmissions.begin(), g_pendingSubmissions.end(), this));
		m_subStatus = status;
	}
	string Submission::GetStatusText() const
	{
		switch(m_subStatus)
		{
		case SubStatus_Accepted:
			return "Accepted";
		case SubStatus_NotExecutable:
			return "Not Executable";
		case SubStatus_Pending:
			return "Pending";
		case SubStatus_PresentationError:
			return "Presentation Error";
		case SubStatus_TimeLimitExceeded:
			return "Time Limit Exceeded";
		case SubStatus_WrongAnswer:
			return "Wrong Answer";
		}
		return "";
	}
	string Submission::GetBase() const
	{
		return m_base;
	}
	SubStatus Submission::AutoTest() //- TODO: Handle Unknown Code Type -
	{
		string target("../scripts/"), sourceFile("./"), execFile("./"), resultFile("./");
		sourceFile.append(m_sourceFile);
		execFile.append(m_base);
		execFile.append(".out");
		resultFile.append(m_base);
		resultFile.append(".log");
		switch(m_codeType)
		{
		case CodeType_C:
			target.append("compile_c.sh");
			break;
		case CodeType_CPP:
			target.append("compile_cpp.sh");
			break;
		case CodeType_Java:
			target.append("compile_java.sh");
			break;
		}
		target.append(" ");
		target.append(sourceFile);
		target.append(" ");
		target.append(execFile);
		target.append(" ");
		target.append(resultFile);
		system(target.c_str());

		if(fileSize(resultFile.c_str()))
			cout << "Compilation Error" << endl;
	//	fileDelete(resultFile.c_str());
	//	fileDelete(execFile.c_str());

		return SubStatus_Accepted;
	}
}
