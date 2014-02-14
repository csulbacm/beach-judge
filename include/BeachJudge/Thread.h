#ifndef _BEACHJUDGE_THREAD_H_
#define _BEACHJUDGE_THREAD_H_

//- Standard Library -
#include <stdlib.h>

//- Beach Judge -
#include <BeachJudge/Config.h>

namespace beachjudge
{
	class Thread
	{
		#if BEACHJUDGE_USEPOSIXTHREAD
			unsigned long m_pThread;
		#endif
		#if BEACHJUDGE_USEWINTHREAD
			void *m_threadHandle;
		#endif

		void *(*m_process)(void *);

	public:
		static void Exit(void *retVal = 0);
		static void DeleteDead();

		Thread(void *(*process)(void *));
		~Thread();

		void Start(void *arg = 0, void *attributes = 0);
		void Join(void **ret = 0);
		bool IsRunning() const;
		void End();
	};
}

#endif
