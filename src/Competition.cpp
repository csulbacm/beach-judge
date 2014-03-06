//- Standard Library -
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Competition.h>
#include <BeachJudge/Problem.h>
#include <BeachJudge/Question.h>

#ifdef _WIN32
	#define SPRINTF sprintf_s
#else
	#define SPRINTF	sprintf
#endif

using namespace std;

namespace beachjudge
{
	Competition *g_currentCompetition = 0;

	Competition *Competition::GetCurrent()
	{
		return g_currentCompetition;
	}
	Competition *Competition::CreateFromFile(string file)
	{
		if(!fileExists(file.c_str()))
			return 0;

		Competition *competition = new Competition();

		string in;
		ifstream inFile(file.c_str());

		while(getline(inFile, in, '\t'))
		{
			if(!in.compare("duration"))
			{
				getline(inFile, in);
				competition->m_duration = atoi(in.c_str());
			}
			else if(!in.compare("endTime"))
			{
				getline(inFile, in);
				competition->m_endTimeMS = atol(in.c_str());
				competition->m_startTimeMS = competition->m_endTimeMS - competition->m_duration; //- TODO: Clean this up -
			}
			else if(!in.compare("questions"))
			{
				getline(inFile, in, '\t');
				Problem *problem = Problem::LookupByID(atoi(in.c_str()));
				getline(inFile, in);
				stringstream questionsStream(in);
				while(getline(questionsStream, in, '\t'))
				{
					stringstream questionInfo(in);
					unsigned short tID;
					bool answered;
					questionInfo >> tID >> answered;
					string text, answer;
					Team *team = Team::LookupByID(tID);
					getline(questionsStream, text, '\t');
					if(answered)
						getline(questionsStream, answer, '\t');
					Question *question = Question::Create(text, team, problem);
					if(answered)
						question->Answer(answer);
				}
			}
			else if(!in.compare("submissions"))
			{
				getline(inFile, in);
				stringstream submissionsStream(in);
				while(getline(submissionsStream, in, '\t'))
				{
					stringstream submissionInfo(in);
					unsigned short sID, tID, pID, cType, sStatus;
					unsigned long timeMS;
					submissionInfo >> sID >> tID >> pID >> cType >> sStatus >> timeMS;
					Team *team = Team::LookupByID(tID);
					Problem *problem = Problem::LookupByID(pID);
					Submission::Create(team, problem, (CodeType)cType, timeMS, (SubStatus)sStatus, sID);
				}
			}
		}

		inFile.close();

		return competition;
	}
	Competition *Competition::Create(unsigned long duration)
	{
		Competition *competition = new Competition();
		competition->m_duration = duration;
		return competition;
	}

	Competition::Competition()
	{
		m_startTimeMS = m_endTimeMS = m_duration = 0;
		m_isRunning = false;
	}
	Competition::~Competition()
	{
	}
	void Competition::Start()
	{
		if(m_startTimeMS == 0)
		{
			m_startTimeMS = getRealTimeMS();
			m_endTimeMS = m_startTimeMS + m_duration;
		}
		m_isRunning = true;
	}
	void Competition::Stop()
	{
		m_isRunning = false;
		m_startTimeMS = 0;
		m_endTimeMS = 0;
	}
	bool Competition::IsRunning() const
	{
		return m_isRunning;
	}
	unsigned long Competition::GetTimeLeft() const
	{
		unsigned long currTimeMS = getRealTimeMS();
		if(currTimeMS > m_endTimeMS)
			return 0;
		else
			return m_endTimeMS - currTimeMS;
	}
	unsigned long Competition::GetDuration() const
	{
		return m_duration;
	}
	unsigned long Competition::CalculateTimeScore(unsigned long timeMS)
	{
		return m_duration - (m_endTimeMS - timeMS);
	}
	void Competition::SetCurrent()
	{
		g_currentCompetition = this;
	}
	void Competition::SaveToFile(string file)
	{
		createFolder(filePath(file.c_str()).c_str());
		ofstream outFile(file.c_str());
		outFile << "duration\t" << m_duration << endl;
		outFile << "endTime\t" << m_endTimeMS << endl;
		map<Problem *, vector<Question *> > &questionByProblem = Question::GetQuestionsByProblem();
		for(map<Problem *, vector<Question *> >::iterator it = questionByProblem.begin(); it != questionByProblem.end(); it++)
		{
			Problem *problem = it->first;
			outFile << "questions\t" << problem->GetID();
			vector<Question *> &questions = it->second;
			for(vector<Question *>::iterator itB = questions.begin(); itB != questions.end(); itB++)
			{
				Question *question = *itB;
				bool isAnswered = question->IsAnswered();
				outFile << "\t" << question->GetTeam()->GetID() << " " << isAnswered << "\t" << question->GetText();
				if(isAnswered)
					outFile << "\t" << question->GetAnswer();
			}
			outFile << endl;
		}
		map<unsigned short, Submission *> &submissions = Submission::GetSubmissionsByID();
		if(submissions.size())
		{
			outFile << "submissions";
			for(map<unsigned short, Submission *>::iterator it = submissions.begin(); it != submissions.end(); it++)
			{
				Submission *submission = it->second;
				Team *team = submission->GetTeam();
				Problem *problem = submission->GetProblem();
				outFile << "\t" << submission->GetID() << " " << team->GetID() << " " << problem->GetID() << " " << submission->GetCodeType() << " " << submission->GetStatus() << " " << submission->GetTimeMS();
			}
			outFile << endl;
		}

		outFile.close();
	}
	void Competition::ClearAll()
	{
		map<unsigned short, Submission *> &submissions = Submission::GetSubmissionsByID();
		if(submissions.size())
			for(map<unsigned short, Submission *>::iterator it = submissions.begin(); it != submissions.end(); it++)
			{
				Submission *submission = it->second;
				fileDelete(submission->GetSourceFile().c_str());
			}
		Question::Cleanup();
		Submission::Cleanup();
		fileDelete("compo/compo.txt");
	}
}
