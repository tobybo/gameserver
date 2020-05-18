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

void CSocket::event_accept(lp_connection_t oldc)
{
	struct sockaddr mysockaddr;
	socklen_t socklen;
	int err;
	int level;
	int s;
	static int use_accept4 = 1;
	lp_connection_t new_pConn;
	do{
		if(use_accept4)
		{
			s = accept4(oldc->fd,&mysockaddr,&socklen,SOCK_NONBLOCK);
		}
		else
		{
			s = accept(oldc->fd,&mysockaddr,&socklen);
		}
		if(s == -1)
		{
			err = errno;
			if(err == EAGAIN)
			{
				return;
			}
			level = LOG;
			if(err == ECONNABORTED)
			{
				level = ERROR;
			}
			else if(err == EMFILE || err == ENFILE )
			{
				level = ERROR;
			}
			log(level,"[SOCKE] event_accept err, errno: %d",err);
			if(use_accept4 && err == ENOSYS)
			{
				use_accept4 = 0;
				continue;
			}
			return;
		}//end if(s == -1)
		new_pConn = get_connection(s);
		if(new_pConn == nullptr)
		{
			if(close(s) == -1)
			{
				log(ERROR,"[SOCKET] event_accept close fd err");
			}
			return;
		}
		memcpy(&new_pConn->s_sockaddr,&mysockaddr,socklen);
		if(!use_accept4)
		{
			if(!setnonblocking(s))
			{
				log(ERROR,"[SOCKET] event_accept setnonblocking err");
				closeconnection(new_pConn);
				return;
			}
		}
		new_pConn->listening = oldc->listening;

		new_pConn->rhandler = &CSocket::read_request_handler;
		new_pConn->whandler = &CSocket::write_request_handler;
		if(epoll_oper_event(s,
					EPOLL_CTL_ADD,
					EPOLLIN|EPOLLRDHUP,
					0,
					new_pConn) == -1)
		{
			closeconnection(new_pConn);
			return;
		}
		//g_socket.addConn(new_pConn); 连接池从list改成数组后 可以直接用索引查找
		break;
	}while(1);

	return;
}
