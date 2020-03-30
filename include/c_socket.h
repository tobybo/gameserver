#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <netinet/in.h>
#include <sys/socket.h>

#include <vector>

#define LISTEN_BACKLOG 511

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

	int fd;
	lp_listening_t listening;

	uint64_t iCurrsequence;
	struct sockaddr s_sockaddr;

	event_handler_pt rhandler;
	event_handler_pt whandler;

	uint32_t events; //epoll

	//收包相关

	//发包相关

	//回收相关

	lp_connection_t next;

};

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

	int epoll_process_events(int timer);
private:
	void ReadConf();
	bool open_listening_sockets();
	bool setnonblocking(int sockfd);

private:
	int m_ListenPortCount;    //监听的端口数量

	std::vector<lp_listening_t> m_ListenSocketList; //监听套接字列表
};

#endif
