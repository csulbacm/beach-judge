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
		problem->LoadTestSets();
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

		string oldBase(base);
		oldBase.append(oldBuff);
		string oldDesc(oldBase), oldTest(oldBase), oldTestSets(oldBase), oldTests(oldBase);
		oldTest.append("-sample.zip");
		oldDesc.append(".pdf");
		oldTestSets.append("-tests");
		oldTests.append("-tests.txt");

		string newBase(base);
		newBase.append(newBuff);
		string newDesc(newBase), newTest(newBase), newTestSets(newBase), newTests(newBase);
		newTest.append("-sample.zip");
		newDesc.append(".pdf");
		newTestSets.append("-tests");
		newTests.append("-tests.txt");

		fileDelete(newTest.c_str());
		fileDelete(newDesc.c_str());
		fileDelete(newTests.c_str());
		//TODO: Delete test sets?
		g_problemsByID.erase(m_id);
		m_id = id;
		g_problemsByID[m_id] = this;

		fileRename(oldDesc.c_str(), newDesc.c_str());
		fileRename(oldTest.c_str(), newTest.c_str());
		fileRename(oldTestSets.c_str(), newTestSets.c_str());
		fileRename(oldTests.c_str(), newTests.c_str());
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
	void Problem::SaveTestSets()
	{
		char idBuff[8];
		memset(idBuff, 0, 8);
		SPRINTF(idBuff, "%d", m_id);

		string file("compo/problems/");
		file.append(idBuff);
		file.append("-tests.txt");

		ofstream outFile(file.c_str());
		for(map<unsigned short, Problem::TestSet *>::iterator it = m_testSetMap.begin(); it != m_testSetMap.end(); it++)
		{
			Problem::TestSet *testSet = it->second;
			outFile << testSet->GetName() << endl;
		}
		outFile.close();
	}
	void Problem::LoadTestSets()
	{
		char idBuff[8];
		memset(idBuff, 0, 8);
		SPRINTF(idBuff, "%d", m_id);

		string file("compo/problems/");
		file.append(idBuff);
		file.append("-tests.txt");

		ifstream inFile(file.c_str());
		string name;
		while(getline(inFile, name))
			TestSet::Create(this, name);
	}
	Problem::TestSet *Problem::TestSet::Create(Problem *problem, string name, string inFile, string outFile)
	{
		unsigned short id = problem->m_testSetMap.size() + 1;

		Problem::TestSet *testSet = new Problem::TestSet();
		problem->m_testSetMap[id] = testSet;
		testSet->m_id = id;
		testSet->m_problem = problem;
		testSet->m_name = name;

		char tidBuff[8], pidBuff[8];
		memset(tidBuff, 0, 8);
		memset(pidBuff, 0, 8);
		SPRINTF(tidBuff, "%d", id);
		SPRINTF(pidBuff, "%d", problem->GetID());

		string base = "compo/problems/";
		base.append(pidBuff);
		base.append("-tests/");

		string tBase(base);
		tBase.append(tidBuff);
		string tIn(tBase), tOut(tBase);
		tIn.append(".in");
		tOut.append(".out");

		testSet->m_inFile = tIn;
		testSet->m_outFile = tOut;

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
		char oldBuff[8], newBuff[8], pidBuff[8];
		memset(oldBuff, 0, 8);
		memset(newBuff, 0, 8);
		memset(pidBuff, 0, 8);
		SPRINTF(oldBuff, "%d", m_id);
		SPRINTF(newBuff, "%d", id);
		SPRINTF(pidBuff, "%d", m_problem->GetID());

		string base = "compo/problems/";
		base.append(pidBuff);
		base.append("-tests/");

		string oldBase(base);
		oldBase.append(oldBuff);
		string oldIn(oldBase), oldOut(oldBase);
		oldIn.append(".in");
		oldOut.append(".out");

		string newBase(base);
		newBase.append(newBuff);
		string newIn(newBase), newOut(newBase);
		newIn.append(".in");
		newOut.append(".out");

		fileDelete(newIn.c_str());
		fileDelete(newOut.c_str());

		m_problem->m_testSetMap.erase(m_id);
		m_id = id;
		m_problem->m_testSetMap[m_id] = this;
		m_inFile = newIn;
		m_outFile = newOut;

		fileRename(oldIn.c_str(), newIn.c_str());
		fileRename(oldOut.c_str(), newOut.c_str());
	}
	string Problem::TestSet::GetName() const
	{
		return m_name;
	}
	void Problem::TestSet::SetName(string name)
	{
		m_name = name;
	}
	string Problem::TestSet::GetInFile() const
	{
		return m_inFile;
	}
	string Problem::TestSet::GetOutFile() const
	{
		return m_outFile;
	}
}
