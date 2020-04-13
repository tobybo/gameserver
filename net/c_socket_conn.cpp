#include <strings.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ngx_funs.h"
#include "c_socket.h"
#include "global.h"
#include "macro.h"
#include "c_memory.h"
#include "c_lockmutex.h"

using std::string;

connection_s::connection_s(){
	iCurrsequence = 0;
	pthread_mutex_init(&logicPorcMutex,NULL);
}

connection_s::~connection_s(){
	pthread_mutex_destroy(&logicPorcMutex);
}

void connection_s::getOneToUse(){
	++iCurrsequence;

	curStat = _PKG_HD_INIT;
	irecvlen = sizeof(COMM_PKG_HEADER);
	precvbuf = dataHeadInfo;

	precvMemPointer = nullptr;


	events = 0;
}

void connection_s::putOneToFree(){
	++iCurrsequence;
}

//---------------------------------
lp_connection_t CSocket::create_one_connection()
{
	lp_connection_t p_Conn;
	CMemory* mem_instance = CMemory::GetInstance();
	p_Conn = (lp_connection_t)mem_instance->AllocMemory(sizeof(connection_s),true);
	p_Conn = new(p_Conn) connection_s();
	//p_Conn->connection_s();
	return p_Conn;
}

//初始化连接池
void CSocket::initconnection(){
	lp_connection_t p_Conn;
	size_t lenConns = sizeof(connection_s);
	CMemory* mem_instance = CMemory::GetInstance();
	for(int i = 0;i <= m_worker_connections;i++)
	{
		p_Conn = (lp_connection_t)mem_instance->AllocMemory(lenConns,true);
		p_Conn->getOneToUse();
		m_connectionList.push_back(p_Conn);
		m_freeconnectionList.push_back(p_Conn);
	}
	m_free_connection_n = m_total_connection_n = m_connectionList.size();
}

//最终回收连接池
void CSocket::clearconnection(){
	lp_connection_t p_Conn = nullptr;
	CMemory *mem_instance = CMemory::GetInstance();
	while(!m_connectionList.empty())
	{
		p_Conn = m_connectionList.front();
		m_connectionList.pop_front();
		p_Conn->~connection_s();
		mem_instance->FreeMemory(p_Conn);
	}
}

lp_connection_t CSocket::get_connection(int isock)
{
	lp_connection_t p_Conn;

	if(!m_freeconnectionList.empty())
	{
		p_Conn = m_freeconnectionList.front();
		m_freeconnectionList.pop_front();
		++m_free_connection_n;
		p_Conn->getOneToUse();
		p_Conn->fd = isock;
		return p_Conn;
	}
	p_Conn = create_one_connection();
	p_Conn->getOneToUse();
	m_connectionList.push_back(p_Conn);
	++m_total_connection_n;
	p_Conn->fd = isock;
	return p_Conn;
}

void CSocket::free_connection(lp_connection_t pConn)
{
	CLock lock(&m_connectionMutex);

	pConn->putOneToFree();

	m_freeconnectionList.push_back(pConn);

	++m_free_connection_n;
}

void CSocket::inRecyConnectQueue(lp_connection_t pConn)
{
	CLock lock(&m_recyconnqueueMutex);

	time(&pConn->iRecyTime);

	++pConn->iCurrsequence;

	m_recyconnectionList.push_back(pConn);

	++m_recy_connection_n;
}

//回收数据线程
void* CSocket::ServerRecyConnectionThread(void* threadData){
	ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
	CSocket *pSocketObj = pThread->_pThis;

	time_t currtime;
	int err;
	std::list<lp_connection_t>::iterator pos,posend;
	lp_connection_t pConn;
	while(1)
	{
		usleep(200*1000); //微秒
		if(pSocketObj->m_recy_connection_n > 0)
		{
			time(&currtime);
			err = pthread_mutex_lock(&pSocketObj->m_recyconnqueueMutex);
			if(err!=0) log(ERROR,"[SOCKET] ServerRecyConnectionThread pthread_mutex_t err");
lblRRTT:
			pos = pSocketObj->m_recyconnectionList.begin();
			posend = pSocketObj->m_recyconnectionList.end();
			for(;pos!=posend;pos++)
			{
				pConn = *pos;
				if(
					(pConn->iRecyTime + pSocketObj->m_RecyConnectionWaitTime > currtime) && (g_stopEvent == 0)
				)
				{
					continue; //未到回收时间,略过
				}//end if
				if(pConn->iThrowsendCount != 0)
				{
					log(ERROR," ServerRecyConnectionThread throwsendcount err");
				}//end if
				pSocketObj->m_recyconnectionList.erase(pos);
				--pSocketObj->m_recy_connection_n;

				pSocketObj->free_connection(pConn);
				goto lblRRTT;
			}//end for
			err = pthread_mutex_unlock(&pSocketObj->m_recyconnqueueMutex);
			if(err!=0) log(ERROR,"[SOCKET] ServerRecyConnectionThread pthread_mutex_t err");
		}//end if
		if(g_stopEvent == 1)
		{
			if(pSocketObj->m_recy_connection_n > 0)
			{
				err = pthread_mutex_lock(&pSocketObj->m_recyconnqueueMutex);
				if(err!=0) log(ERROR,"[SOCKET] ServerRecyConnectionThread pthread_mutex_t 2 err");
lblRRTT2:
				pos = pSocketObj->m_recyconnectionList.begin();
				posend = pSocketObj->m_recyconnectionList.end();
				for(;pos!=posend;pos++)
				{
					pConn = *pos;
					pSocketObj->m_recyconnectionList.erase(pos);
					--pSocketObj->m_recy_connection_n;

					pSocketObj->free_connection(pConn);
					goto lblRRTT2;
				}//end for
				err = pthread_mutex_unlock(&pSocketObj->m_recyconnqueueMutex);
				if(err!=0) log(ERROR,"[SOCKET] ServerRecyConnectionThread pthread_mutex_t 2 err");
			}//end if
			break;
		}//end if
	}//end while

	return (void *)0;
}

void CSocket::closeconnection(lp_connection_t pConn)
{
	int fd = pConn->fd;
	free_connection(pConn);
	if(fd != -1 && (close(fd) == -1))
	{
		log(ERROR,"[SOCKET] closeconnection, close fd err, fd: %d",fd);
	}
	return;
}
