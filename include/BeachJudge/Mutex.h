#ifndef _BEACHJUDGE_MUTEX_H_
#define _BEACHJUDGE_MUTEX_H_

//- Beach Judge -
#include <BeachJudge/Config.h>

namespace beachjudge
{
	class Mutex
	{
		bool m_isAlive;
		#if BEACHJUDGE_USEPOSIXTHREAD
			void *m_pMutex;
		#endif
		#if BEACHJUDGE_USEWINTHREAD
			void *m_mutexHandle;
		#endif

	public:
		Mutex();
		~Mutex();

		void Init();
		void Destroy();
		void Lock();
		void Unlock();
		bool IsAlive() const;
	};
}

#endif
