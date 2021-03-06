#include <strings.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/tcp.h>
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

CSocket::CSocket(){
	m_ListenPortCount = 1;
	m_worker_connections = 1;
	m_RecyConnectionWaitTime = 60;
	m_epollhandle = -1;

	m_iLenPkgHeader = sizeof(COMM_PKG_HEADER);
	m_iLenMsgHeader = sizeof(STRUC_MSG_HEADER);

	return;
}

CSocket::~CSocket(){

}

void CSocket::ReadConf(){
	CConfig *config_instance = CConfig::getInstance();
	m_ListenPortCount = std::stoi((*config_instance)["LISTENPORT_COUNT"]); m_worker_connections = std::stoi((*config_instance)["WORKER_CONNECTIONS"]);
}

bool CSocket::setnonblocking(int sockfd){
	int nb = 1;
	if(ioctl(sockfd,FIONBIO,&nb) == -1){
		return false;
	}
	return true;
}

bool CSocket::Initialize(){
	ReadConf();
	return true;
	//return open_listening_sockets(); 每个进程单独执行 避免惊群
}

bool CSocket::open_listening_sockets(){
	int isock;
	struct sockaddr_in serv_addr;
	int iport;
	char strinfo[100];

	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	CConfig* config_instance = CConfig::getInstance();
	for(int i = 0; i<m_ListenPortCount; i++){
		isock = socket(AF_INET,SOCK_STREAM,0);
		if(isock == -1){
			log(ERROR,"[SOCKET] open_listening_sockets err,num: %d",i);
			return false;
		}
		int reuseaddr = 1;
		if(setsockopt(isock,SOL_SOCKET,SO_REUSEADDR,(const void*) &reuseaddr, sizeof(reuseaddr)) == -1){
			log(ERROR,"[SOCKET] open_listening_sockets setopt err, num: %d",i);
			close(isock);
			return false;
		}
		//避免惊群
		if(setsockopt(isock,SOL_SOCKET,SO_REUSEPORT,(const void*) &reuseaddr, sizeof(reuseaddr)) == -1){
			log(ERROR,"[SOCKET] open_listening_sockets setopt1 err, num: %d",i);
			close(isock);
			return false;
		}
		//避免拥塞控制的nagle算法增加消息延迟
		if(setsockopt(isock,IPPROTO_TCP,TCP_NODELAY,(const void*) &reuseaddr, sizeof(reuseaddr)) == -1){
			log(ERROR,"[SOCKET] open_listening_sockets setopt2 err, num: %d",i);
			close(isock);
			return false;
		}
		if(setnonblocking(isock) == false){
			log(ERROR,"[SOCKET] open_listening_sockets setlok err, num: %d",i);
			close(isock);
			return false;
		}
		strinfo[0] = 0;
		sprintf(strinfo,"LISTENPORT%d",i);
		iport = std::stoi((*config_instance)[string(strinfo)]);
		serv_addr.sin_port = htons(iport);

		if(bind(isock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
			log(ERROR,"[SOCKET] open_listening_sockets bind err, num: %d",i);
			close(isock);
			return false;
		}

		if(listen(isock, LISTEN_BACKLOG)==-1){
			log(ERROR,"[SOCKET] open_listening_sockets listen err, num: %d",i);
			close(isock);
			return false;
		}

		lp_listening_t p_listensocketitem = new listening_t;
		bzero(p_listensocketitem,sizeof(listening_t));
		p_listensocketitem -> port = iport;
		p_listensocketitem -> fd = isock;
		p_listensocketitem -> connection = nullptr;
		log(LOG,"[SOCKET] open_listening_sockets succ, num: %d, port: %d",i,iport);
		m_ListenSocketList.push_back(p_listensocketitem);
	}
	if(m_ListenSocketList.size() <= 0)
	{
		return false;
	}
	return true;
}

bool CSocket::Initialize_subporc(){
	//发消息互斥量初始化
	if(pthread_mutex_init(&m_sendMessageQueueMutex, NULL) != 0)
	{
		log(ERROR,"[SOCKET] Initialize_subporc err: init sendMsgQueueMutex");
		return false;
	}
	//连接相关互斥量初始化
	if(pthread_mutex_init(&m_connectionMutex, NULL) != 0)
	{
		log(ERROR,"[SOCKET] Initialize_subporc err: init connectionMutex");
		return false;
	}
	//连接回收队列相关互斥量初始化
	if(pthread_mutex_init(&m_recyconnqueueMutex, NULL) != 0)
	{
		return false;
	}
	//发消息队列信号量初始化
	if(sem_init(&m_semEventSendQueue,0,0) == -1){
		log(ERROR,"[SOCKET] Initialize_subporc err: init sem_t");
		return false;
	}

	//创建发消息线程
	int err; //创建线程一般不通过errno返回错误
	ThreadItem *pSendQueue = new ThreadItem(this);
	m_threadVector.push_back(pSendQueue);
	err = pthread_create(&pSendQueue->_Handle,NULL,ServerSendQueue,pSendQueue);
	if(err != 0)
		return false;

	ThreadItem *pRecyconn; //专门用来回收连接的线程
	m_threadVector.push_back(pRecyconn = new ThreadItem(this));
	err = pthread_create(&pRecyconn->_Handle,NULL,ServerRecyConnectionThread,pSendQueue);
	if(err != 0)
	{
		return false;
	}
	return true;
}

void CSocket::Shutdown_subproc(){
	if(sem_post(&m_semEventSendQueue) == -1)
	{
		log(ERROR,"[STOP] Shutdown_subproc, sem_post err");
	}

	auto pos = m_threadVector.begin();
	for(;pos < m_threadVector.end();pos++)
	{
		pthread_join((*pos)->_Handle,NULL);
	}
	for(pos = m_threadVector.begin();pos < m_threadVector.end();pos++)
	{
		if(*pos)
			delete *pos;
	}
	m_threadVector.clear();

	clearMsgSendQueue();
	clearconnection();

	pthread_mutex_destroy(&m_connectionMutex);
	pthread_mutex_destroy(&m_sendMessageQueueMutex);
	pthread_mutex_destroy(&m_recyconnqueueMutex);
	sem_destroy(&m_semEventSendQueue);
}

int CSocket::epoll_process_events(int timer){
	int events = epoll_wait(m_epollhandle,m_events,MAX_EVENTS,timer);
	if(events == -1)
	{
		if(errno == EINTR)
		{
			log(LOG,"[EPOLL] epoll_process_events EINTR");
			return 1;
		}
		else
		{
			log(ERROR,"[EPOLL] epoll_process_events errno: %d",errno);
			return 0;
		}
	}
	if(events == 0)
	{
		if(timer != -1)
		{
			return 1; //超时 没有事件来
		}
		//一直等待 不应该到来而没有事件
		log(ERROR,"[EPOLL] epoll_process_events no events");
		return 0;
	}
	//惊群?
	lp_connection_t pConn;
	uint32_t revent;
	for(int i = 0;i < events;i++)
	{
		pConn = (lp_connection_t)(m_events[i].data.ptr);

		revent = m_events[i].events;
		if(revent & EPOLLIN)
		{
			(this->*(pConn->rhandler))(pConn);
		}
		if(revent & EPOLLOUT)
		{
			if(revent & (EPOLLERR|EPOLLHUP|EPOLLRDHUP))
			{
				--pConn->iThrowsendCount;
			}
			else
			{
				(this->*(pConn->whandler))(pConn);
			}
		}
	}

	return 1;
}

void CSocket::threadRecvProcFunc(char *pMsgBuf){

	return;
}

//发数据线程
void* CSocket::ServerSendQueue(void *threadData){
	ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
	CSocket *pSocketObj = pThread->_pThis;
	int err;
	std::list<char *>::iterator pos,pos2,posend;

	char *pMsgBuf;
	LPSTRUC_MSG_HEADER pMsgHeader;
	LPCOMM_PKG_HEADER  pPkgHeader;
	lp_connection_t    pConn;
	unsigned short     itmp;
	ssize_t            sendsize;

	CMemory* mem_instance = CMemory::GetInstance();

	while(g_stopEvent == 0)
	{
		if(sem_wait(&pSocketObj->m_semEventSendQueue) == -1)
		{
			if(errno != EINTR)
				log(ERROR,"[MSG_SEND] ServerSendQueue sem_wait err, errno: %d",errno);
		}
		if(g_stopEvent != 0)
			break;

		if(pSocketObj->m_iSendMsgQueueCount > 0) //原子的
		{
			err = pthread_mutex_lock(&pSocketObj->m_sendMessageQueueMutex);
			if(err != 0) log(ERROR,"[MSG_SEND] ServerSendQueue lock err, err: %d",err);
			pos = pSocketObj->m_MsgSendQueue.begin();
			posend = pSocketObj->m_MsgSendQueue.end();
			while(pos != posend)
			{
				pMsgBuf = *pos;
				pMsgHeader = (LPSTRUC_MSG_HEADER)pMsgBuf;
				pPkgHeader = (LPCOMM_PKG_HEADER)(pMsgBuf + pSocketObj->m_iLenMsgHeader);
				pConn = pMsgHeader->pConn;

				if(pConn->iCurrsequence != pMsgHeader->iCurrsequence)
				{
					pos2 = pos;
					pos++;
					pSocketObj->m_MsgSendQueue.erase(pos2);
					--pSocketObj->m_iSendMsgQueueCount;
					mem_instance->FreeMemory(pMsgBuf);
					continue;
				}

				if(pConn->iThrowsendCount > 0)
				{
					pos++;
					continue;
				}
				//可以发送
				pConn->psendMemPointer = pMsgBuf;
				pos2 = pos;
				pos++;
				pSocketObj->m_MsgSendQueue.erase(pos2);
				--pSocketObj->m_iSendMsgQueueCount;
				pConn->psendbuf = (char*)pPkgHeader; //从包头开始发 不发消息头
				itmp = ntohs(pPkgHeader->pkgLen); //包头+包体的长度
				pConn->isendlen = itmp;

				log(INFO,"[MSG_SEND] ServerSendQueue ready send");

				sendsize = pSocketObj->sendproc(pConn,pConn->psendbuf,pConn->isendlen);
				if(sendsize > 0)
				{
					if(sendsize >= pConn->isendlen)
					{
						//发送完数据
						mem_instance->FreeMemory(pConn->psendMemPointer);
						pConn->psendMemPointer = nullptr;
						pConn->iThrowsendCount = 0;
						log(INFO,"[MSG_SEND] ServerSendQueue send all succ");
					}
					else
					{
						pConn->psendbuf += sendsize;
						pConn->isendlen -= sendsize;
						++pConn->iThrowsendCount;

						if(pSocketObj->epoll_oper_event(pConn->fd,
									EPOLL_CTL_MOD, //在监听套接字接连接进来的时候就加入了epoll的红黑树中
									EPOLLOUT, //默认LT模式
									0,
									pConn) == -1)
						{
							log(ERROR,"[MSG_SEND] ServerSendQueue, epoll_mod err!!!!!!, fd: %d",pConn->fd);
						}
						log(LOG,"[MSG_SEND] ServerSendQueue, epoll_mod succ");
					}
					continue;
				}
				else if(sendsize == 0)
				{
					//对端断开连接 丢弃包
					mem_instance->FreeMemory(pConn->psendMemPointer);
					pConn->psendMemPointer = nullptr;
					pConn->iThrowsendCount = 0;
					continue;
				}
				else if(sendsize == -1)
				{
					++pConn->iThrowsendCount;

					if(pSocketObj->epoll_oper_event(pConn->fd,
								EPOLL_CTL_MOD, //在监听套接字接连接进来的时候就加入了epoll的红黑树中
								EPOLLOUT, //默认LT模式
								0,
								pConn) == -1)
					{
						log(ERROR,"[MSG_SEND] ServerSendQueue, epoll_mod err2!!!!!!, fd: %d",pConn->fd);
					}
					log(LOG,"[MSG_SEND] ServerSendQueue, epoll_mod succ2");
				}
				else
				{
					//其他错误 也认为是对端断开
					mem_instance->FreeMemory(pConn->psendMemPointer);
					pConn->psendMemPointer = nullptr;
					pConn->iThrowsendCount = 0;
					continue;
				}
			}//end while
			err = pthread_mutex_unlock(&pSocketObj->m_sendMessageQueueMutex);
			if(err != 0) log(ERROR,"[MSG_SEND] ServerSendQueue unlock err, err: %d",err);
		}//end if(count > 0)
	}//end if while(g_stopEvent == 0)

	return (void *)0;
}

//epoll初始化
int CSocket::epoll_process_init(){
	m_epollhandle = epoll_create(m_worker_connections);
	if(m_epollhandle == -1)
	{
		log(ERROR,"[SOCKET] epoll_process_init err");
		exit(2);
	}
	initconnection();

	auto pos = m_ListenSocketList.begin();
	for(;pos < m_ListenSocketList.end(); pos++)
	{
		lp_connection_t pConn = get_connection((*pos)->fd);
		if(pConn == nullptr)
		{
			log(ERROR,"[SOCKET] epoll_process_init err conn");
			exit(2);
		}
		pConn->listening = (*pos);
		(*pos)->connection = pConn;
		pConn->rhandler = &CSocket::event_accept;
		if(epoll_oper_event((*pos)->fd,
							EPOLL_CTL_ADD,
							EPOLLIN|EPOLLRDHUP,
							0,
							pConn) == -1)
		{
			exit(2);
		}
		log(INFO,"[EPOLL] epoll_process_init succ, lsport: %d,lsfd: %d",(*pos)->port,(*pos)->fd);
	}
	return 1;
}

int CSocket::epoll_oper_event(int fd,
						 uint32_t eventtype,
						 uint32_t flag,
						 int baction,
						 lp_connection_t pConn)
{
	struct epoll_event ev;
	bzero(&ev,sizeof(struct epoll_event));
	if(eventtype == EPOLL_CTL_ADD)
	{
		ev.events = flag;
		pConn->events = flag;
	}
	else if(eventtype == EPOLL_CTL_MOD)
	{
		ev.events = pConn->events;
		if(baction == 0)
		{
			//增加标记
			ev.events |= flag;
		}
		else if(baction == 1)
		{
			//移除标记
			ev.events &= ~flag;
		}
		else
		{
			//覆盖
			ev.events = flag;
		}
		pConn->events = ev.events;
	}
	else
	{
		//close socket的时候会自动从红黑树移除
		return 1;
	}
	ev.data.ptr = (void *)pConn;
	if(epoll_ctl(m_epollhandle,eventtype,fd,&ev) == -1)
	{
		log(ERROR,"[EPOLL] event_poll_ctl err, op: %d, fd: %d, flag: %d",
				eventtype,fd,flag);
		return -1;
	}
	return 1;
}

void CSocket::msgSend(char* psendbuf)
{
	CLock lock(&m_sendMessageQueueMutex);
	m_MsgSendQueue.push_back(psendbuf);
	++m_iSendMsgQueueCount;

	if(sem_post(&m_semEventSendQueue) == -1)
	{
		log(ERROR,"[SEND_MSG] msgSend sem_post err");
	}

	return;
}

void CSocket::clearMsgSendQueue()
{
	char* sTmpMempoint;
	CMemory* mem_instance = CMemory::GetInstance();

	while(!m_MsgSendQueue.empty())
	{
		sTmpMempoint = m_MsgSendQueue.front();
		m_MsgSendQueue.pop_front();
		mem_instance->FreeMemory(sTmpMempoint);
	}
}
