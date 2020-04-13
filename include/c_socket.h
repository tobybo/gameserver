#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <atomic>

#include <vector>
#include <list>

#include "net_comm.h"

#define LISTEN_BACKLOG 511
#define MAX_EVENTS     512

typedef struct listening_s listening_t,*lp_listening_t;
typedef struct connection_s connection_t,*lp_connection_t;
typedef class CSocket CSocket;

typedef void (CSocket::*event_handler_pt)(lp_connection_t c);

struct listening_s
{
	int port;
	int fd;
	lp_connection_t connection;
};

struct connection_s
{
	connection_s();
	virtual ~connection_s();
	void getOneToUse();
	void putOneToFree();

	int fd;
	lp_listening_t listening;

	unsigned instance:1;
	uint64_t iCurrsequence;
	struct sockaddr s_sockaddr;

	event_handler_pt rhandler;
	event_handler_pt whandler;

	uint32_t events; //epoll

	//收包相关
	pthread_mutex_t logicPorcMutex;
	unsigned char curStat;
	char dataHeadInfo[_DATA_BUFSIZE_];
	char* precvbuf;
	unsigned int irecvlen;
	char* precvMemPointer;

	//发包相关
	std::atomic<int> iThrowsendCount;

	//回收相关
	time_t iRecyTime; //放到回收队列的时间

	lp_connection_t next;

};

typedef struct
{
	lp_connection_t pConn;
	uint64_t iCurrsequence;
}STRUC_MSG_HEADER,*LPSTRUC_MSG_HEADER;

class CSocket
{
public:
	CSocket();
	virtual ~CSocket();
	virtual bool Initialize();
	virtual bool Initialize_subporc();
	virtual void Shutdown_subproc();

public:
	//收包
	virtual void threadRecvProcFunc(char *pMsgBuf);

	//epoll相关函数
	int epoll_process_init(); //epoll功能初始化
	int epoll_process_events(int timer);
	int epoll_oper_event(int fd,
						 uint32_t eventtype,
						 uint32_t flag,
						 int baction,
						 lp_connection_t pConn);
private:
	void ReadConf();
	bool open_listening_sockets();
	bool setnonblocking(int sockfd);

	//一些业务处理的handler
	void event_accept(lp_connection_t oldc); //监听套接字的读事件，将新的客户端连接接进来
	void read_request_handler(lp_connection_t pConn); //读事件响应
	void write_request_handler(lp_connection_t pConn); //写事件响应

	//收包发包的一些工具函数
	ssize_t recvproc(lp_connection_t pConn,char* buff,ssize_t bufflen);
	void wait_request_handler_proc_p1(lp_connection_t pConn);
	void wait_request_handler_proc_last(lp_connection_t pConn);

	//连接池或者连接相关
	lp_connection_t create_one_connection();
	void initconnection();
	void clearconnection();
	void closeconnection(lp_connection_t);
	lp_connection_t get_connection(int isock); //从连接池中获取一个空闲连接
	void free_connection(lp_connection_t pConn); //归还参数pConn所代表的连接到连接池
	void inRecyConnectQueue(lp_connection_t pConn);

	//线程相关函数
	static void* ServerSendQueue(void* threadData);
	static void* ServerRecyConnectionThread(void* threadData);

protected:
	size_t m_iLenPkgHeader;
	size_t m_iLenMsgHeader;

private:
	struct ThreadItem
	{
		pthread_t _Handle;
		CSocket *_pThis;
		bool ifrunning;

		ThreadItem(CSocket *pthis):_pThis(pthis),ifrunning(false){}
		~ThreadItem(){}
	};

	int m_ListenPortCount;    //监听的端口数量
	int m_worker_connections; //epoll连接的最大项数 这里是客户端连上来的最大数量
	int m_epollhandle;        //epoll_create返回的句柄 epoll事件集合句柄

	std::vector<lp_listening_t> m_ListenSocketList; //监听套接字列表
	struct epoll_event m_events[MAX_EVENTS];

	//连接相关变量
	std::list<lp_connection_t> m_connectionList; //连接列表[连接池]
	std::list<lp_connection_t> m_freeconnectionList; //空闲连接列表[连接池]
	std::list<lp_connection_t> m_recyconnectionList; //回收连接列表[连接池]
	std::atomic<int> m_total_connection_n; //连接池总连接数
	std::atomic<int> m_free_connection_n; //连接池空闲总连接数
	std::atomic<int> m_recy_connection_n; //回收连接池总连接数
	pthread_mutex_t m_connectionMutex; //连接相关互斥量
	pthread_mutex_t m_recyconnqueueMutex; //连接回收队列相关的互斥量
	pthread_mutex_t m_sendMessageQueueMutex; //处理发消息线程相关的信号量
	int m_RecyConnectionWaitTime; //回收连接延迟时间

	std::vector<ThreadItem*> m_threadVector; //各个线程的容器
	sem_t           m_semEventSendQueue; //处理发消息队列的信号量 无名信号量
};

#endif
