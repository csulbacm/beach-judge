#ifndef _BEACHJUDGE_COMPETITION_H_
#define _BEACHJUDGE_COMPETITION_H_

//- Beach Judge -
#include <BeachJudge/Problem.h>

namespace beachjudge
{
	class Competition
	{
		unsigned long m_startTime;
		bool m_isRunning;

	public:
		void Start();
		void Stop();
		bool IsRunning() const;
	};
}

#endif
