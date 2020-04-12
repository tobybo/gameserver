//发包/收包
#include <strings.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "config.h"
#include "ngx_funs.h"
#include "c_socket.h"
#include "global.h"
#include "macro.h"
#include "c_memory.h"
#include "c_lockmutex.h"

using std::string;
using std::cout;
using std::endl;

void CSocket::read_request_handler(lp_connection_t pConn)
{
	ssize_t n;
	char buff[1025];
	n = recv(pConn->fd,buff,1024,0);
	if(n == 0)
	{
		//客户端关闭连接
		if(close(pConn->fd) == -1)
		{
			log(ERROR,"[SOCKET] read_request_handler close fd err, fd: %d",pConn->fd);
		}
		else
		{
			log(INFO,"[SOCKET] read_request_handler close fd succ, fd: %d",pConn->fd);
		}
		inRecyConnectQueue(pConn);
		return;
	}
	//cout<<"[SOCKET] read_request_handler: "<<buff<<endl;
	log(INFO,"[SOCKET] read_request_handler: %s",buff);
}

void CSocket::write_request_handler(lp_connection_t pConn)
{
	char test_str[] = "hi,i'm a linux server.";
	send(pConn->fd,(void*)test_str,sizeof(test_str),0);
	//cout<<"[SOCKET] write_request_handler~ "<<endl;
	log(INFO,"[SOCKET] write_request_handler~");
}
