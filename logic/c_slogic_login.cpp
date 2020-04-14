#include <stdlib.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <atomic>

#include <vector>
#include <list>
#include <map>

#include "macro.h"
#include "net_comm.h"
#include "c_slogic.h"
#include "c_crc32.h"
#include "global.h"
#include "ngx_funs.h"

bool CLogicSocket::onPlyaerRgist( lp_connection_t pConn,
								  LPSTRUC_MSG_HEADER pMsgHeader,
								  char* pPkgBody,
							      unsigned short iBodyLength)
{
	log(INFO,"[LOGIC_LOGIN] onPlyaerRgist succ, buff: %s",pPkgBody);
	return true;
}

bool CLogicSocket::onPlyaerLogin( lp_connection_t pConn,
								  LPSTRUC_MSG_HEADER pMsgHeader,
								  char* pPkgBody,
							      unsigned short iBodyLength)
{
	log(INFO,"[LOGIC_LOGIN] onPlyaerLogin succ, buff: %s",pPkgBody);
	return true;
}
