#ifndef _C_THREADPOOL_H_
#define _C_THREADPOOL_H_

#include "c_socket.h"

#include <vector>
#include <pthread.h>
#include <atomic>
#include <list>

class CThreadPool
{
public:
	CThreadPool();
	~CThreadPool();
public:
	bool Create(int threadNum);
	void StopAll();

	void inMsgRecvQueueAndSignal(char *buf);
	void Call();

private:
	static void* ThreadFunc(void *threadData);
	void clearMsgRecvQueue();

private:
	struct ThreadItem{
		pthread_t _Handle;
		CThreadPool *_pThis;
		bool ifrunning;

		ThreadItem(CThreadPool *pthis):_pThis(pthis),ifrunning(false){}
		~ThreadItem(){}
	};


private:
	static pthread_cond_t  m_pthreadCond;  //条件变量
	static bool            m_shutdown;     //线程退出标志 false 不退出

	int                    m_iThreadNum;   //要创建的线程数量

	std::atomic<int>       m_iRunningThreadNum; //运行中的线程数
	time_t                 m_iLastEmgTime; //上次发生线程不够用的时间

	std::vector<ThreadItem *> m_threadVector; //线程结构数组

public:
	static pthread_mutex_t m_pthreadMutex; //互斥量
	std::list<char*>       m_MsgRecvQueue; //接收数据消息队列
	int                    m_iRecvMsgQueueCount; //接收消息队列大小
};


#endif
