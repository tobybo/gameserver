#include <stdlib.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <atomic>
#include <string.h>

#include <vector>
#include <list>
#include <map>
#include <string>

#include "macro.h"
#include "net_comm.h"
#include "c_slogic.h"
#include "c_crc32.h"
#include "global.h"
#include "ngx_funs.h"
#include "logic_comm.h"

using std::string;

bool CLogicSocket::onPlyaerRgist( lp_connection_t pConn,
								  LPSTRUC_MSG_HEADER pMsgHeader,
								  char* pPkgBody,
							      unsigned short iBodyLength)
{
	LPMSGSTR_ST_PLY_REGIST regist_stru = new MSGSTR_ST_PLY_REGIST;
	regist_stru->playerId = 1;
	string account = "account1";
	memcpy(regist_stru->playerAccount,account.c_str(),sizeof(account));
	msgSend(pConn,ST_ON_PLY_REGIST,(char*)regist_stru,(unsigned short)sizeof(MSGSTR_ST_PLY_REGIST));
	delete regist_stru; //内存已经被copy到sendbuf
	log(INFO,"[LOGIC_LOGIN] onPlyaerRgist succ");
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
