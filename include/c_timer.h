#ifndef _C_TIMER_H_
#define _C_TIMER_H_

#include <map>
#include <vector>
#include <list>
#include <string>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "macro.h"
#include "c_min_heap.h"

typedef struct _str_events str_events,*lpstr_events;
typedef struct _str_timer str_timer,*lpstr_timer;

struct _str_events
{
	int eventId;
	int int_param1;
	int int_param2;
	std::string string_param1;
	std::string string_param2;
	_str_events(int eId,int iP1,int iP2,std::string& sP1,std::string& sP2);
};

struct _str_timer
{
	u_long msec;
	std::vector<lpstr_events> events;
};

class CTimer
{
	private:
		CTimer();
	public:
		~CTimer();
	public:
		static CTimer* GetInstance()
		{
			if(!m_instance)
			{
				m_instance = new CTimer();
				static GarHuishou clr;
			}

			return m_instance;
		};
		class GarHuishou{
			public:
				~GarHuishou(){
					if(m_instance)
					{
						delete m_instance;
						m_instance = nullptr;
					}
				}
		};

	public:
		void addTimer(u_long cd_msec, int timer_id, int int_param1, int int_param2, std::string string_param1, std::string string_param2);
		int getTimerCount();
		lpstr_events getOneEvent();
		void semPost();

	public:
		static void* threadFunCheckTimer(void*);

	private:
		static CMinHeap<u_long>* m_heap_timers; //小顶堆用于取最近的时间
		static std::map<u_long,lpstr_timer> m_map_timers; //map用于查找某个时间是否已经注册过事件

		static std::list<lpstr_events> m_over_timers; //可以执行的timer

		//增加timer事件 只需要加锁 m_mutex_map_timers
		//抛出timer事件 只需要加锁 m_mutex_over_timers
		//检查timer事件 需要先加锁 map 取出一个str_timer后 再加锁 over 再解锁
		//循环检查直到小顶堆的顶部时间 > 当前时间 或者 小顶堆为空
		static pthread_mutex_t m_mutex_map_timers; //用于增加timer事件和检查timer事件的互斥量
		static pthread_mutex_t m_mutex_over_timers; //用于检查timer事件和抛出timer事件的互斥量
		static sem_t m_sem;

	private:
		static CTimer* m_instance;

};

#endif
