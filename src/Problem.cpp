//- Standard Library -
#include <map>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <string>
#include <algorithm>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Problem.h>
#include <BeachJudge/Team.h>

#ifdef _WIN32
	#define SPRINTF sprintf_s
#else
	#define SPRINTF	sprintf
#endif

using namespace std;

namespace beachjudge
{
	map<unsigned short, Problem *> g_problemsByID;

	Problem *g_workingProblem = 0;
	bool SolverComp(Team *teamA, Team *teamB)
	{
		float scoreA = teamA->GetScore(g_workingProblem);
		float scoreB = teamB->GetScore(g_workingProblem);
		unsigned short penA = teamA->GetPenalties(g_workingProblem);
		unsigned short penB = teamB->GetPenalties(g_workingProblem);
		if(scoreA == 0.f && scoreB == 0.f)
			return penA > penB;
		if(scoreA == 0.f)
			return true;
		if(scoreB == 0.f)
			return false;
		return scoreA < scoreB;
	}

	Problem *Problem::Create(string name)
	{
		unsigned short id = g_problemsByID.size() + 1;
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
	void Problem::LoadFromFile(const char *file)
	{
		ifstream inFile(file);
		string name;
		while(getline(inFile, name))
			Create(name);
	}
	void Problem::SaveToFile(const char *file)
	{
		ofstream outFile(file);
		for(map<unsigned short, Problem *>::iterator it = g_problemsByID.begin(); it != g_problemsByID.end(); it++)
		{
			Problem *problem = it->second;
			outFile << problem->GetName() << endl;
		}
		outFile.close();
	}
	void Problem::ClearSolvers()
	{
		for(map<unsigned short, Problem *>::iterator it = g_problemsByID.begin(); it != g_problemsByID.end(); it++)
			it->second->m_solvers.clear();
	}

	Problem::Problem()
	{
		m_id = 0;
	}
	Problem::~Problem()
	{
		g_problemsByID.erase(m_id);
		if(m_id < g_problemsByID.size())
			for(unsigned short i = m_id; i <= g_problemsByID.size(); i++)
				g_problemsByID[i + 1]->SetID(i);
	}
	string Problem::GetName() const
	{
		return m_name;
	}
	void Problem::SetName(string name)
	{
		m_name = name;
	}
	unsigned short Problem::GetID() const
	{
		return m_id;
	}
	void Problem::AddSolver(Team *team)
	{
		if(find(m_solvers.begin(), m_solvers.end(), team) == m_solvers.end())
		{
			m_solvers.push_back(team);
			g_workingProblem = this;
		}
		sort(m_solvers.begin(), m_solvers.end(), SolverComp);
	}
	void Problem::RemoveSolver(Team *team)
	{
		m_solvers.erase(find(m_solvers.begin(), m_solvers.end(), team));
	}
	vector<Team *> *Problem::GetSolvers()
	{
		return &m_solvers;
	}
	void Problem::SetID(unsigned short id)
	{
		char oldBuff[8], newBuff[8];
		memset(oldBuff, 0, 8);
		memset(newBuff, 0, 8);
		SPRINTF(oldBuff, "%d", m_id);
		SPRINTF(newBuff, "%d", id);

		string base = "compo/problems/";
		string oldDesc(base), oldTest(base);
		oldTest.append(oldBuff);
		oldTest.append("-sample.zip");
		oldDesc.append(oldBuff);
		oldDesc.append(".pdf");

		string newDesc(base), newTest(base);
		newTest.append(newBuff);
		newTest.append("-sample.zip");
		newDesc.append(newBuff);
		newDesc.append(".pdf");

		fileDelete(newTest.c_str());
		fileDelete(newDesc.c_str());
		g_problemsByID.erase(m_id);
		m_id = id;
		g_problemsByID[m_id] = this;

		fileRename(oldDesc.c_str(), newDesc.c_str());
		fileRename(oldTest.c_str(), newTest.c_str());
	}
	map<unsigned short, Problem::TestSet *> *Problem::GetTestSets()
	{
		return &m_testSetMap;
	}
	Problem::TestSet *Problem::LookupTestSetByID(unsigned short id)
	{
		if(m_testSetMap.count(id))
			return m_testSetMap[id];
		return 0;
	}


	Problem::TestSet *Problem::TestSet::Create(Problem *problem, string name, string inFile, string outFile)
	{
		unsigned short id = problem->m_testSetMap.size() + 1;

		Problem::TestSet *testSet = new Problem::TestSet();
		problem->m_testSetMap[id] = testSet;
		testSet->m_id = id;
		testSet->m_problem = problem;
		testSet->m_name = name;
		return testSet;
	}

	Problem::TestSet::TestSet()
	{
		m_id = 0;
		m_problem = 0;
	}
	Problem::TestSet::~TestSet()
	{
		m_problem->m_testSetMap.erase(m_id);
		if(m_id < m_problem->m_testSetMap.size())
			for(unsigned short i = m_id; i <= m_problem->m_testSetMap.size(); i++)
				m_problem->m_testSetMap[i + 1]->SetID(i);
	}
	unsigned short Problem::TestSet::GetID() const
	{
		return m_id;
	}
	void Problem::TestSet::SetID(unsigned short id)
	{
/*		
		char oldBuff[8], newBuff[8];
		memset(oldBuff, 0, 8);
		memset(newBuff, 0, 8);
		SPRINTF(oldBuff, "%d", m_id);
		SPRINTF(newBuff, "%d", id);

		string base = "compo/problems/";
		string oldDesc(base), oldTest(base);
		oldTest.append(oldBuff);
		oldTest.append("-sample.zip");
		oldDesc.append(oldBuff);
		oldDesc.append(".pdf");

		string newDesc(base), newTest(base);
		newTest.append(newBuff);
		newTest.append("-sample.zip");
		newDesc.append(newBuff);
		newDesc.append(".pdf");

		fileDelete(newTest.c_str());
		fileDelete(newDesc.c_str());

		fileRename(oldDesc.c_str(), newDesc.c_str());
		fileRename(oldTest.c_str(), newTest.c_str());
		*/

		m_problem->m_testSetMap.erase(m_id);
		m_id = id;
		m_problem->m_testSetMap[m_id] = this;
	}
	string Problem::TestSet::GetName() const
	{
		return m_name;
	}
	void Problem::TestSet::SetName(string name)
	{
		m_name = name;
	}
}
