//- Standard Library -
#include <map>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Team.h>

//- SHA1 -
#include <sha1.h>

#define PENALTY 1200.f

using namespace std;

namespace beachjudge
{
	string g_teamDatabase = "teams.txt";
	map<string, Team *> g_teamByNameMap;
	map<unsigned short, Team *> g_teamByIDMap;
	vector<Team *> g_teamsByScore, g_teamsByName;
	
	string sha1Convert(string str)
	{
		unsigned char buff[20];
		char hex[40];
		sha1::calc(str.c_str(), str.length(), buff);
		sha1::toHexString(buff, hex);
		return string(hex);
	}
	bool ByScoreComp(Team *teamA, Team *teamB)
	{
		if(teamA->GetNumSolutions() > teamB->GetNumSolutions())
			return true;
		if(teamA->GetNumSolutions() < teamB->GetNumSolutions())
			return false;
		if(teamA->GetTotalScore() < teamB->GetTotalScore())
			return true;
		if(teamA->GetTotalScore() > teamB->GetTotalScore())
			return false;
		return teamA->GetName().compare(teamB->GetName()) < 0;
	}
	bool ByNameComp(Team *teamA, Team *teamB)
	{
		return teamA->GetName().compare(teamB->GetName()) < 0;
	}

	map<unsigned short, Team *> &Team::GetTeamsByID()
	{
		return g_teamByIDMap;
	}
	Team *Team::LookupByID(unsigned short id)
	{
		if(g_teamByIDMap.count(id))
			return g_teamByIDMap[id];
		return 0;
	}
	Team *Team::LookupByName(std::string name)
	{
		if(g_teamByNameMap.count(name))
			return g_teamByNameMap[name];
		return 0;
	}
	Team *Team::Create(string name, string password, unsigned short id, bool isJudge)
	{
		Team *team = LookupByName(name);
		if(!team)
		{
			team = new Team();
			team->m_name = name;
			g_teamByNameMap[name] = team;

			if(id == 0 && !isJudge)
			{
				do
				{
					id = (unsigned short)rand();
				}
				while(id == 0 || g_teamByIDMap.count(id));
			}
			team->m_id = id;
			g_teamByIDMap[team->m_id] = team;
			if(team->m_id != 0)
			{
				g_teamsByScore.push_back(team);
				g_teamsByName.push_back(team);
			}
			sort(g_teamsByScore.begin(), g_teamsByScore.end(), ByScoreComp);
			sort(g_teamsByName.begin(), g_teamsByName.end(), ByNameComp);
		}

		team->m_password = sha1Convert(password);
		team->m_isJudge = isJudge;
		return team;
	}
	void Team::SetDatabase(string file)
	{
		g_teamDatabase = file;
	}
	void Team::SaveToDatabase()
	{
		createFolder(filePath(g_teamDatabase.c_str()).c_str());
		ofstream outFile(g_teamDatabase.c_str());
		for(map<unsigned short, Team *>::iterator it = g_teamByIDMap.begin(); it != g_teamByIDMap.end(); it++)
		{
			Team *team = it->second;
			outFile << team->m_id << '\t' << team->m_name << '\t' << team->m_password << endl;
		}
		outFile.close();
	}
	void Team::LoadFromDatabase()
	{
		if(fileExists(g_teamDatabase.c_str()))
		{
			ifstream inFile(g_teamDatabase.c_str());
			string idStr, name, password;
			while(getline(inFile, idStr, '\t'))
			{
				getline(inFile, name, '\t');
				getline(inFile, password);

				unsigned short id = atoi(idStr.c_str());
				Team *team = LookupByID(id);
				if(!team)
					team = Create(name, "", id);
				team->SetName(name);
				team->m_password = password;

				g_teamByIDMap[id] = team;
				g_teamByNameMap[name] = team;
			}
			inFile.close();
		}
	}
	void Team::DeleteAll()
	{
		while(g_teamByIDMap.size())
			delete g_teamByIDMap.begin()->second;
	}
	void Team::SaveScores()
	{
		string scoreDatabase = "compo/scores.txt";
		createFolder(filePath(scoreDatabase.c_str()).c_str());
		ofstream outFile(scoreDatabase.c_str());
		for(map<unsigned short, Team *>::iterator it = g_teamByIDMap.begin(); it != g_teamByIDMap.end(); it++)
		{
			Team *team = it->second;
			if(team->m_penalties.size())
			{
				outFile << team->m_id << " " << team->m_totalScore << " " << team->m_totalPenalties << " " << team->m_numSolutions;
				for(map<Problem *, unsigned short>::iterator itB = team->m_penalties.begin(); itB != team->m_penalties.end(); itB++)
				{
					Problem *problem = itB->first;
					outFile << "\t" << problem->GetID();
					if(team->m_scores.count(problem))
						outFile << " " << team->m_scores[problem];
					else
						outFile << " 0";
					outFile << " " << itB->second;
				}
				outFile << endl;
			}
		}
		outFile.close();
	}
	void Team::LoadScores()
	{
		string scoreDatabase = "compo/scores.txt";
		if(fileExists(scoreDatabase.c_str()))
		{
			ifstream inFile(scoreDatabase.c_str());
			string line, in;
			while(getline(inFile, line))
			{
				stringstream lineStream(line);
				getline(lineStream, in, '\t');
				stringstream inStream(in);
				unsigned short teamID, totalPenalties, numSolutions;
				float totalScore;
				inStream >> teamID >> totalScore >> totalPenalties >> numSolutions;
				Team *team = LookupByID(teamID);
				team->m_totalScore = totalScore;
				team->m_totalPenalties = totalPenalties;
				team->m_numSolutions = numSolutions;
				if(team)
					while(getline(lineStream, in, '\t'))
					{
						stringstream tabStream(in);
						unsigned short problemID, penalties;
						float score;
						tabStream >> problemID >> score >> penalties;
						Problem *problem = Problem::LookupByID(problemID);
						if(problem)
						{
							team->m_scores[problem] = score;
							team->m_penalties[problem] = penalties;
							problem->AddSolver(team);
						}
					}
			}
			sort(g_teamsByScore.begin(), g_teamsByScore.end(), ByScoreComp);
		}
	}
	vector<Team *> *Team::GetTeamsByScore()
	{
		return &g_teamsByScore;
	}
	vector<Team *> *Team::GetTeamsByName()
	{
		return &g_teamsByName;
	}

	Team::Team()
	{
		m_id = m_totalPenalties = m_numSolutions = 0;
		m_totalScore = 0.f;
		m_numActiveSubmissions = 0;
	}
	Team::~Team()
	{
		g_teamByNameMap.erase(m_name);
		g_teamByIDMap.erase(m_id);
		g_teamsByName.erase(find(g_teamsByName.begin(), g_teamsByName.end(), this));
		g_teamsByScore.erase(find(g_teamsByScore.begin(), g_teamsByScore.end(), this));
	}
	void Team::SetPassword(string password)
	{
		m_password = sha1Convert(password);
	}
	bool Team::TestPassword(string password)
	{
		password = sha1Convert(password);
		return m_password.compare(password) == 0;
	}
	void Team::SetName(string name)
	{
		g_teamByNameMap.erase(m_name);
		m_name = name;
		g_teamByNameMap[name] = this;
		sort(g_teamsByName.begin(), g_teamsByName.end(), ByNameComp);
	}
	string Team::GetName() const
	{
		return m_name;
	}
	unsigned short Team::GetID() const
	{
		return m_id;
	}
	unsigned short Team::GetTotalPenalties() const
	{
		return m_totalPenalties;
	}
	unsigned short Team::GetNumSolutions() const
	{
		return m_numSolutions;
	}
	float Team::GetTotalScore() const
	{
		return m_totalScore;
	}
	bool Team::IsJudge() const
	{
		return m_isJudge;
	}
	vector<Submission *> *Team::GetSubmissions()
	{
		return &m_submissions;
	}
	void Team::AddSubmission(Submission *submission)
	{
		m_numActiveSubmissions++;
		m_submissions.push_back(submission);
	}
	void Team::RemoveSubmission(Submission *submission)
	{
		vector<Submission *>::iterator it = find(m_submissions.begin(), m_submissions.end(), submission);
		if(it != m_submissions.end())
		{
			m_numActiveSubmissions--;
			m_submissions.erase(it);
		}
	}
	unsigned short Team::GetNumActiveSubmissions() const
	{
		return m_numActiveSubmissions;
	}
	float Team::GetScore(Problem *problem)
	{
		if(m_scores.count(problem))
			return m_scores[problem];
		return 0.f;
	}
	void Team::AddScore(Problem *problem, float score)
	{
		if(m_scores.count(problem))
			m_scores[problem] += score;
		else
		{
			m_scores[problem] = score;
			m_numSolutions++;
		}
		if(!m_penalties.count(problem))
			m_penalties[problem] = 0;
		m_scores[problem] += m_penalties[problem] * PENALTY;
		m_totalScore += score;

		sort(g_teamsByScore.begin(), g_teamsByScore.end(), ByScoreComp);
	}
	unsigned short Team::GetPenalties(Problem *problem)
	{
		if(m_penalties.count(problem))
			return m_penalties[problem];
		return 0;
	}
	void Team::AddPenalty(Problem *problem)
	{
		if(m_penalties.count(problem))
			m_penalties[problem]++;
		else
			m_penalties[problem] = 1;
		m_totalPenalties++;
	}
	void Team::Reset()
	{
		m_totalPenalties = 0;
		m_numSolutions = 0;
		m_totalScore = 0.f;

		sort(g_teamsByScore.begin(), g_teamsByScore.end(), ByScoreComp);
	}
}
