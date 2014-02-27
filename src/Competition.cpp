//- Standard Library -
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Competition.h>
#include <BeachJudge/Question.h>

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
				competition->m_endTimeS = atoi(in.c_str());
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
		m_startTimeS = m_endTimeS = m_duration = 0;
		m_isRunning = false;
	}
	Competition::~Competition()
	{
	}
	void Competition::Start()
	{
		m_startTimeS = getRealTimeS();
		if(m_endTimeS < m_startTimeS)
			m_endTimeS = m_startTimeS + m_duration;
		m_isRunning = true;
	}
	void Competition::Stop()
	{
		m_isRunning = false;
	}
	bool Competition::IsRunning() const
	{
		return m_isRunning;
	}
	unsigned long Competition::GetTimeLeft() const
	{
		unsigned long currTimeS = getRealTimeS();
		if(currTimeS > m_endTimeS)
			return 0;
		else
			return m_endTimeS - currTimeS;
	}
	unsigned long Competition::GetDuration() const
	{
		return m_duration;
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
		outFile << "endTime\t" << m_endTimeS << endl;
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

		outFile.close();
	}
}
