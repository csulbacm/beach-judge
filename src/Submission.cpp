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
#include <BeachJudge/Thread.h>

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
			submission->m_id = id; //TODO: Clean this up
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
	string Submission::GetStatusText(SubStatus status)
	{
		switch(status)
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

	Submission::Submission()
	{
		m_id = 0;
		m_timeMS = 0;
		m_team = 0;
		m_problem = 0;
		m_codeType = CodeType_Unknown;
		m_subStatus = SubStatus_Pending;
		m_autoTestStatus = SubStatus_Pending;
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
	string Submission::GetBase() const
	{
		return m_base;
	}

	typedef struct TestStruct
	{
		Problem::TestSet *testSet;
		string cmd, resultFile, execFile;
		Thread *thread;
	} TestStruct;
	void *testThreadFunc(void *arg)
	{
		#if BEACHJUDGE_USEPOSIXTHREAD
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
		#endif

		TestStruct *data = (TestStruct *)arg;

		system(data->cmd.c_str());

		fileDelete(data->resultFile.c_str());
		fileDelete(data->execFile.c_str());

		data->thread->End();
		delete data;
		Thread::Exit();
		return 0;
	}
	SubStatus Submission::AutoTest() //TODO: Handle Unknown Code Type
	{
		m_autoTestVerdicts.clear();

		//- Compilation -
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
		case CodeType_Unknown:
			//TODO: Handle unknown type
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
		{
//			cout << "Compilation Error" << endl;
			m_autoTestStatus = SubStatus_NotExecutable;
			fileDelete(resultFile.c_str());
			return m_autoTestStatus;
		}

		//- Run Test Sets -
		map<unsigned short, Problem::TestSet *> *testSetMap = m_problem->GetTestSets();
		unsigned short size = testSetMap->size();

		unsigned short errors = 0;
		bool timeLimitExceeded = false;
		for(unsigned short a = 1; a <= size; a++)
		{
			Problem::TestSet *testSet = testSetMap->operator[](a);

			char cmdBuff[256];
			memset(cmdBuff, 0, 256);
			SPRINTF(cmdBuff, "%s < %s > dummy; diff dummy %s > %s; rm dummy", execFile.c_str(), testSet->GetInFile().c_str(), testSet->GetOutFile().c_str(), resultFile.c_str());
//			print("%s < %s > dummy; diff dummy %s > %s; rm dummy\n", execFile.c_str(), testSet->GetInFile().c_str(), testSet->GetOutFile().c_str(), resultFile.c_str());

			TestStruct *testStruct = new TestStruct;
			testStruct->testSet = testSet;
			testStruct->cmd = string(cmdBuff);
			testStruct->resultFile = resultFile;
			testStruct->execFile = execFile;

			Thread *testThread = new Thread(&testThreadFunc);
			testStruct->thread = testThread;
			
			testThread->Start(testStruct);

			unsigned long long startTime = getRunTimeMS();
			while((getRunTimeMS() - startTime) < 60000) //TODO: Verify if there should be mutexes // Was 3000
			{
				if(!testThread->IsRunning())
					break;
				sleepMS(10);
			}

			if(testThread->IsRunning())
			{
				testThread->Cancel();
				fileDelete(resultFile.c_str());
				fileDelete(execFile.c_str());
				timeLimitExceeded = true;
			}

//			system(cmdBuff);

			if(timeLimitExceeded || fileSize(resultFile.c_str()))
			{
//				cout << "Wrong answer" << endl;
//				break;
				errors++;
				m_autoTestVerdicts[a] = false;
			}
			else
				m_autoTestVerdicts[a] = true;
		}

		if(errors)
		{
			if(timeLimitExceeded)
				m_autoTestStatus = SubStatus_TimeLimitExceeded;
			else
				m_autoTestStatus = SubStatus_WrongAnswer;
//			cout << "Wrong answer" << endl;
			return m_autoTestStatus;
		}

		m_autoTestStatus = SubStatus_Accepted;
		return m_autoTestStatus;
	}
	SubStatus Submission::GetAutoTestStatus() const
	{
		return m_autoTestStatus;
	}
	map<unsigned short, bool> *Submission::GetAutoTestVerdicts()
	{
		return &m_autoTestVerdicts;
	}
}
