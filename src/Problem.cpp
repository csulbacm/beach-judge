//- Standard Library -
#include <map>
#include <cstdlib>
#include <cstring>
#include <cstdio>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Problem.h>

using namespace std;

namespace beachjudge
{
	map<unsigned short, Problem *> g_problemsByID;

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
}
