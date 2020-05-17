#include "c_timer.h"
#include "macro.h"
#include "global.h"
#include "ngx_funs.h"
#include "c_lockmutex.h"

#include <stdlib.h>
#include <pthread.h>
#include <sys/errno.h>
#include <time.h>
#include <map>
#include <list>
#include <semaphore.h>
#include <sys/unistd.h>

CTimer* CTimer::m_instance = nullptr;
CMinHeap<u_long>* CTimer::m_heap_timers = nullptr;
pthread_mutex_t CTimer::m_mutex_map_timers = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CTimer::m_mutex_over_timers = PTHREAD_MUTEX_INITIALIZER;
std::list<lpstr_events> CTimer::m_over_timers;
std::map<u_long,lpstr_timer> CTimer::m_map_timers;
sem_t CTimer::m_sem;

_str_events::_str_events(int eId,int iP1,int iP2,std::string &sP1,std::string &sP2)
{
	eventId = eId;
	int_param1 = iP1;
	int_param2 = iP2;
	string_param1 = sP1;
	string_param2 = sP2;
}

CTimer::CTimer(){
	m_heap_timers = new CMinHeap<u_long>();
	if(sem_init(&m_sem,0,0) == -1)
	{
		log(ERROR,"[TIMER] constructor, sem_init err");
	}
	pthread_t thread_handler;
	int err = pthread_create(&thread_handler,nullptr,&threadFunCheckTimer,nullptr);
	if(err != 0)
	{
		log(ERROR,"[TIMER] pthread_create, err: %d",err);
	}
};

CTimer::~CTimer()
{
	pthread_mutex_destroy(&m_mutex_map_timers);
	pthread_mutex_destroy(&m_mutex_over_timers);
	sem_destroy(&m_sem);
}

void CTimer::addTimer(u_long cd_msec, int timer_id, int int_param1, int int_param2, std::string string_param1, std::string string_param2)
{
	lpstr_events new_event = new str_events(timer_id, int_param1, int_param2, string_param1, string_param2);
	int err;
	err = pthread_mutex_lock(&m_mutex_map_timers);
	if(err != 0)
	{
		log(ERROR,"[TIMER] addTimer, lock err, timer_id: %d",timer_id);
		return;
	}
	u_long msec = gMsec + cd_msec;
	auto iter = m_map_timers.find(msec);
	if(iter != m_map_timers.end())
	{
		//说明存在该时刻的定时器
		iter->second->events.push_back(new_event);
	}
	else
	{
		lpstr_timer new_timer = new str_timer();
		new_timer->msec = msec;
		new_timer->events.push_back(new_event);
		//加入map中
		m_map_timers[msec] = new_timer;
		//加入小顶堆中
		m_heap_timers->pushNode(msec);
		//调试打印
		//m_heap_timers->showTree();
	}
	err = pthread_mutex_unlock(&m_mutex_map_timers);
	if(err != 0)
	{
		log(ERROR,"[TIMER] addTimer, unlock err, timer_id: %d",timer_id);
		return;
	}
}

int CTimer::getTimerCount()
{
	return m_over_timers.size();
}

void* CTimer::threadFunCheckTimer(void* thread_data)
{
	while(!gOn || !gTime)
	{
		sleep(1);
	}
	timespec tsp;
	while(true)
	{
		maketimeoutms(&tsp,gFrameTime);
		int err = sem_timedwait(&m_sem,&tsp);
		if(err != 0)
		{
			if(errno != ETIMEDOUT && errno != EINTR)
				log(ERROR,"[TIMER] sem_timedwait err, err: %d",errno);
		}
		err = pthread_mutex_lock(&m_mutex_map_timers);
		if(err != 0)
			log(ERROR,"[TIMER] thread, lock map err");
		//检查小顶堆
		while(m_heap_timers->checkRoot() == 0)
		{
			u_long msec = m_heap_timers->getRoot();
			if(msec <= gMsec)
			{
				//时间到了 加入 over队列
				m_heap_timers->dropNode();
				err = pthread_mutex_lock(&m_mutex_over_timers);
				if(err != 0)
					log(ERROR,"[TIMER] thread, lock over err");
				auto iter = m_map_timers.find(msec);
				if(iter == m_map_timers.end())
				{
					pthread_mutex_unlock(&m_mutex_over_timers);
					pthread_mutex_unlock(&m_mutex_map_timers);
					goto lbexit;
				}
				lpstr_timer over_timer = iter->second;
				m_map_timers.erase(iter);
				if(over_timer->events.size() > 0)
				{
					auto iter_event = over_timer->events.begin();
					for(; iter_event < over_timer->events.end(); iter_event++)
						m_over_timers.push_back(*iter_event);
				}
				delete over_timer;
				over_timer = nullptr;
				err = pthread_mutex_unlock(&m_mutex_over_timers);
				if(err != 0)
					log(ERROR,"[TIMER] thread, unlock over err");
			}
			else
			{
				break;
			}
		}
		err = pthread_mutex_unlock(&m_mutex_map_timers);
		if(err != 0)
			log(ERROR,"[TIMER] thread, unlock map err");
	}
lbexit:
	log(ERROR,"[TIMER] map and heap dosen't match!!");
	return (void*)0;
}

lpstr_events CTimer::getOneEvent()
{
	CLock lock(&m_mutex_over_timers);
	lpstr_events ret = nullptr;
	if(m_over_timers.size() > 0)
	{
		ret = m_over_timers.front();
		m_over_timers.pop_front();
	}
	return ret;
}

void CTimer::semPost()
{
	if(0 != sem_post(&m_sem))
	{
		log(ERROR,"[TIMER] sem_post err");
	}
}
