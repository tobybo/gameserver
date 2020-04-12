#ifndef _C_LOCKMUTEX_
#define _C_LOCKMUTEX_

#include<pthread.h>

class CLock
{
	public:
		CLock(pthread_mutex_t* pMutex):m_pMutex(pMutex)
		{
			m_pMutex = pMutex;
			pthread_mutex_lock(m_pMutex);
		}
		~CLock()
		{
			pthread_mutex_unlock(m_pMutex);
		}

	private:
		pthread_mutex_t *m_pMutex;
};

#endif
