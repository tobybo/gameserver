#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

#include "global.h"
#include "ngx_funs.h"
#include "c_threadpool.h"
#include "c_memory.h"
#include "macro.h"

//静态成员初始化
pthread_mutex_t CThreadPool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CThreadPool::m_pthreadCond = PTHREAD_COND_INITIALIZER;
bool CThreadPool::m_shutdown = false;

CThreadPool::CThreadPool(){
	m_iRunningThreadNum = 0; //正在运行的线程数量
	m_iLastEmgTime = 0; //上次发生线程不够用的时间
	m_iRecvMsgQueueCount = 0; //收消息队列
}

CThreadPool::~CThreadPool(){
	clearMsgRecvQueue();
}

void CThreadPool::clearMsgRecvQueue(){
	char *sTmpMempoint;
	CMemory *p_memory = CMemory::GetInstance();
	if(!m_MsgRecvQueue.empty()){
		sTmpMempoint = m_MsgRecvQueue.front();
		m_MsgRecvQueue.pop_front();
		p_memory->FreeMemory(sTmpMempoint);
	}
}

bool CThreadPool::Create(int threadNum){
	ThreadItem *pNew;
	int err;

	m_iThreadNum = threadNum;
	for(int i = 0; i<threadNum; i++){
		m_threadVector.push_back(pNew = new ThreadItem(this));
		err = pthread_create(&pNew->_Handle,NULL,ThreadFunc,pNew);
		if(err!=0)
		{
			log(ERROR,"[THREAD] create err, num: %d",i);
			return false;
		}
		else
		{

		}
	}

	auto pos = m_threadVector.begin();
lblfor:
	for(;pos<m_threadVector.end();pos++)
	{
		if((*pos)->ifrunning == false){
			usleep(100 * 1000);
			goto lblfor;
		}
	}
	return true;
}

void* CThreadPool::ThreadFunc(void *threadData){
	//1 取到线程池对象
	ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
	CThreadPool *pThreadPool = pThread->_pThis;

	CMemory* mem_instance = CMemory::GetInstance();
	int err;
	//2 死循环 退出条件为m_shutdown = true
		//1 加锁
		//2 检查是否有消息
			//1 无 阻塞等待线程条件唤醒 并且释放互斥量
			//2 有 从线程池的消息队列取出消息 释放互斥量 然后执行具体消息逻辑
			//循环往复
		//3 释放锁
	pthread_t tid = pthread_self();
	while(true){
		err = pthread_mutex_lock(&m_pthreadMutex);
		if(err!=0)
			log(ERROR,"[THREAD] ThreadFunc lockerr, tid: %d, err: %d",tid,err);
		while((pThreadPool->m_MsgRecvQueue.size()==0)&&m_shutdown==false)
		{
			if(pThread->ifrunning == false)
			{
				pThread->ifrunning = true;
			}
			pthread_cond_wait(&m_pthreadCond,&m_pthreadMutex);
		}
		if(m_shutdown)
		{
			pthread_mutex_unlock(&m_pthreadMutex);
			break;
		}

		char *jobbuff = pThreadPool->m_MsgRecvQueue.front();
		pThreadPool->m_MsgRecvQueue.pop_front();
		--pThreadPool->m_iRecvMsgQueueCount;

		err = pthread_mutex_unlock(&m_pthreadMutex);
		if(err!=0)
			log(ERROR,"[THREAD] ThreadFunc unlockerr, tid: %d, err: %d",tid,err);

		++pThreadPool->m_iRunningThreadNum;
		g_socket.threadRecvProcFunc(jobbuff);
		mem_instance->FreeMemory(jobbuff);
		--pThreadPool->m_iRunningThreadNum;
	}
	return (void*)0;
}
