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

	Problem *Problem::Create(unsigned short id, string name)
	{
		if(g_problemsByID.count(id))
			return g_problemsByID[id];
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
		unsigned short id;
		string name;
		while(inFile >> id)
		{
			getline(inFile, name);
			Create(id, name.substr(1));
		}
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
	}
	string Problem::GetName() const
	{
		return m_name;
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
}
