//- Standard Library -
#include <map>
#include <fstream>
#include <string>
#include <iostream>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Team.h>

//- SHA1 -
#include <sha1.h>

using namespace std;

namespace beachjudge
{
	string g_teamDatabase = "teams.txt";
	map<string, Team *> g_teamByNameMap;
	map<unsigned short, Team *> g_teamByIDMap;
	
	string sha1Convert(string str)
	{
		unsigned char buff[20];
		char hex[40];
		sha1::calc(str.c_str(), str.length(), buff);
		sha1::toHexString(buff, hex);
		return string(hex);
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
	unsigned short Team::GetNumActiveSubmissions() const
	{
		return m_numActiveSubmissions;
	}
}
