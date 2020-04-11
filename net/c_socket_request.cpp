//发包/收包
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
using std::cout;
using std::endl;

void CSocket::read_request_handler(lp_connection_t pConn)
{
	char buff[1025];
	recv(pConn->fd,buff,1024);
	cout<<"[SOCKET] read_request_handler: "<<buff<<endl;
}

void CSocket::write_request_handler(lp_connection_t pConn)
{
	string test_str = "hi,i'm a linux server."
	send(pConn,test_str,sizeof(test_str),0);
	cout<<"[SOCKET] write_request_handler~ "<<endl;
}
