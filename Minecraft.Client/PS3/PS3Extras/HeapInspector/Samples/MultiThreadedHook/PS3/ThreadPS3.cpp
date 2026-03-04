#include <pthread.h>
#include "../IThread.h"

void* InternalThread(void* a_UserData)
{
	ThreadFunction function = (ThreadFunction)a_UserData;
	function();

	pthread_exit(0);
	return 0;
}

class ThreadPS3 : public IThread
{
public:
	virtual void Fork(ThreadFunction a_Function)
	{
		pthread_create(&m_Thread, 0, InternalThread, (void*)a_Function);
	}

	virtual void Join()
	{
		void* threadResult;
		pthread_join(m_Thread, &threadResult);
	}

private:
	pthread_t m_Thread;
};


IThread* CreateThread()
{
	return new ThreadPS3();
}

void DestroyThread(IThread* a_Thread)
{
	delete a_Thread;
}

