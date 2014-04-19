//- Standard Library -
#include <cassert>
#include <vector>

#ifdef _WIN32
	#include <windows.h>
#endif

//- Beach Judge -
#include <BeachJudge/Thread.h>

#if BEACHJUDGE_USEPOSIXTHREAD
	//- POSIX -
	#include <pthread.h>
#endif

using namespace std;

namespace beachjudge
{
	vector<Thread *> g_deadThreads;

	void Thread::DeleteDead()
	{
		for(vector<Thread *>::iterator it = g_deadThreads.begin(); it != g_deadThreads.end(); it++)
			delete *it;
		g_deadThreads.clear();
	}
	Thread::Thread(void *(*process)(void *))
	{
		m_process = process;

		#if BEACHJUDGE_USEPOSIXTHREAD
			m_pThread = 0;
		#endif

		#if BEACHJUDGE_USEWINTHREAD
			m_threadHandle = 0;
		#endif
	}
	Thread::~Thread()
	{
		#if BEACHJUDGE_USEPOSIXTHREAD
			if(m_pThread)
		#endif

		#if BEACHJUDGE_USEWINTHREAD
			if(m_threadHandle)
		#endif
				Join(NULL);
	}
	void Thread::Start(void *arg, void *attributes)
	{
		#if BEACHJUDGE_USEPOSIXTHREAD
			assert(m_pThread == 0);
			pthread_create((pthread_t *)&m_pThread, (const pthread_attr_t *)attributes, m_process, arg);
		#endif

		#if BEACHJUDGE_USEWINTHREAD
			assert(m_threadHandle == 0);
			m_threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)m_process, arg, 0, 0);
		#endif
	}
	void Thread::Exit(void *retVal)
	{
		#if BEACHJUDGE_USEPOSIXTHREAD
			pthread_exit(retVal);
		#endif

		#if BEACHJUDGE_USEWINTHREAD
			ExitThread((DWORD)retVal);
		#endif
	}
	void Thread::Join(void **ret)
	{
		#if BEACHJUDGE_USEPOSIXTHREAD
			assert(m_pThread != 0);
			pthread_join(m_pThread, ret);
			m_pThread = 0;
		#endif

		#if BEACHJUDGE_USEWINTHREAD
			assert(m_threadHandle != 0);
			WaitForSingleObject((HANDLE)m_threadHandle, INFINITE);
			CloseHandle((HANDLE)m_threadHandle);
			m_threadHandle = 0;
		#endif
	}
	bool Thread::IsRunning() const
	{
		#if BEACHJUDGE_USEPOSIXTHREAD
			return m_pThread != 0;
		#endif

		#if BEACHJUDGE_USEWINTHREAD
			return m_threadHandle != 0;
		#endif

		return false;
	}
	void Thread::End()
	{
		g_deadThreads.push_back(this);
	}
	void Thread::Cancel()
	{
		End();

		#if BEACHJUDGE_USEPOSIXTHREAD
			pthread_cancel(m_pThread);
		#endif
	}
}
