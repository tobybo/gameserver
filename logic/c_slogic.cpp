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

typedef bool (CLogicSocket::*handler)( lp_connection_t pConn,
									   LPSTRUC_MSG_HEADER pMsgHeader,
									   char* pPkgBody,
									   unsigned short iBodyLength);

static std::map<unsigned short,handler> msgMap = {
	//{0x01,NULL},
	{0x1000,&CLogicSocket::onPlyaerRgist},
	{0x1001,&CLogicSocket::onPlyaerLogin},
};

CLogicSocket::CLogicSocket(){

}

CLogicSocket::~CLogicSocket(){

}

bool CLogicSocket::Initialize(){
	bool bParentInit = CSocket::Initialize();
	return bParentInit;
}

//处理具体消息
void CLogicSocket::threadRecvProcFunc(char *pMsgBuf){
	LPSTRUC_MSG_HEADER pMsgHeader = (LPSTRUC_MSG_HEADER)pMsgBuf;
	LPCOMM_PKG_HEADER  pPkgHeader = (LPCOMM_PKG_HEADER)(pMsgBuf + m_iLenMsgHeader);
	void* pPkgBody;
	unsigned short pkglen = ntohs(pPkgHeader->pkgLen);

	if(m_iLenPkgHeader == pkglen)
	{
		if(pPkgHeader->crc32 != 0)
		{
			return;
		}
		pPkgBody = nullptr;
	}
	else
	{
		pPkgBody = (void*)(pMsgBuf + m_iLenMsgHeader + m_iLenPkgHeader);
		int calccrc = CCRC32::GetInstance()->Get_CRC((unsigned char*)pPkgBody, pkglen - m_iLenPkgHeader);
		if(calccrc != ntohl(pPkgHeader->crc32))
		{
			log(ERROR,"[CRC32] threadRecvProcFunc crc32 err");
			return;
		}
	}

	unsigned short imsgCode = ntohs(pPkgHeader->msgCode);
	lp_connection_t pConn = pMsgHeader->pConn;

	if(pConn->iCurrsequence != pMsgHeader->iCurrsequence)
	{
		return; //连接已经被废弃或者复用了 丢弃该类包
	}

	if(msgMap[imsgCode] == NULL)
	{
		log(ERROR,"[CLOGIC] threadRecvProcFunc no defiened msg, code: %d",imsgCode);
		return;
	}

	(this->*msgMap[imsgCode])(pConn,pMsgHeader,(char*)pPkgBody,pkglen-m_iLenPkgHeader);
	return;
}

