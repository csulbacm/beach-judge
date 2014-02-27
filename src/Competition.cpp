//- Standard Library -
#include <iostream>
#include <fstream>
#include <cstdlib>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Competition.h>

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
		outFile.close();
	}
}
