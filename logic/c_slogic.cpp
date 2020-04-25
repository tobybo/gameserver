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

#include "macro.h"
#include "net_comm.h"
#include "c_slogic.h"
#include "c_crc32.h"
#include "global.h"
#include "ngx_funs.h"
#include "logic_comm.h"
#include "c_memory.h"

typedef bool (CLogicSocket::*handler)( lp_connection_t pConn,
									   LPSTRUC_MSG_HEADER pMsgHeader,
									   char* pPkgBody,
									   unsigned short iBodyLength);

typedef std::map<unsigned short,handler> _msgMap;
static _msgMap msgMap = {
	//{0x01,NULL},
	{PT_ON_PLY_REGIST  ,  &CLogicSocket::onPlyaerRgist},
	{PT_ON_PLY_LOGIN   ,  &CLogicSocket::onPlyaerLogin},
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

	//auto iter = msgMap.find(imsgCode);
	//(this->*(iter->second))(pConn,pMsgHeader,(char*)pPkgBody,pkglen-m_iLenPkgHeader);

	(this->*msgMap[imsgCode])(pConn,pMsgHeader,(char*)pPkgBody,pkglen-m_iLenPkgHeader);
	return;
}

void CLogicSocket::msgSend(lp_connection_t pConn,unsigned short msgCode,char* pPkgBody,unsigned short bodyLen)
{
	if(bodyLen + m_iLenPkgHeader > _PKG_MAX_LENGTH - 1000)
	{
		log(ERROR,"[MSG_SEND] msgSend err, pkglen is too longer, msgCode: %d, bodyLen: %d",msgCode,bodyLen);
		return;
	}
	CMemory* mem_instance = CMemory::GetInstance();
	char *psendbuf = (char*)mem_instance->AllocMemory(bodyLen + m_iLenMsgHeader + m_iLenPkgHeader,true);
	LPSTRUC_MSG_HEADER pMsgHeader = (LPSTRUC_MSG_HEADER)psendbuf;
	pMsgHeader->pConn = pConn;
	pMsgHeader->iCurrsequence = pConn->iCurrsequence;
	LPCOMM_PKG_HEADER pPkgHeader = (LPCOMM_PKG_HEADER)(psendbuf + m_iLenMsgHeader);
	pPkgHeader->msgCode = htons(msgCode);
	pPkgHeader->pkgLen = htons(m_iLenPkgHeader + bodyLen);

	CCRC32* crc32_instance = CCRC32::GetInstance();
	pPkgHeader->crc32 = crc32_instance->Get_CRC((unsigned char*)pPkgHeader,m_iLenPkgHeader + bodyLen);

	memcpy(psendbuf + m_iLenMsgHeader + m_iLenPkgHeader, pPkgBody, bodyLen);

	this->CSocket::msgSend(psendbuf);
}

int CLogicSocket::getJobBuff(char*& jobbuff, int& jobpos)
{
	if(g_threadpool.m_iRecvMsgQueueCount <= 0)
		return 0;

	char* pMsgBuf = g_threadpool.m_MsgRecvQueue.front();
	jobpos = m_iLenMsgHeader + m_iLenPkgHeader;

	LPSTRUC_MSG_HEADER pMsgHeader = (LPSTRUC_MSG_HEADER)pMsgBuf;
	LPCOMM_PKG_HEADER  pPkgHeader = (LPCOMM_PKG_HEADER)(pMsgBuf + m_iLenMsgHeader);
	void* pPkgBody;
	unsigned short pkglen = ntohs(pPkgHeader->pkgLen);

	if(m_iLenPkgHeader == pkglen)
	{
		if(pPkgHeader->crc32 != 0)
		{
			return 0;
		}
	}
	else
	{
		pPkgBody = (void*)(pMsgBuf + jobpos);
		int calccrc = CCRC32::GetInstance()->Get_CRC((unsigned char*)pPkgBody, pkglen - m_iLenPkgHeader);
		if(calccrc != ntohl(pPkgHeader->crc32))
		{
			log(ERROR,"[CRC32] threadRecvProcFunc crc32 err");
			return 0;
		}
	}

	unsigned short imsgCode = ntohs(pPkgHeader->msgCode);
	lp_connection_t pConn = pMsgHeader->pConn;

	if(pConn->iCurrsequence != pMsgHeader->iCurrsequence)
	{
		return 0; //连接已经被废弃或者复用了 丢弃该类包
	}

	g_threadpool.m_MsgRecvQueue.pop_front();
	--g_threadpool.m_iRecvMsgQueueCount;

	int err = pthread_mutex_unlock(&g_threadpool.m_pthreadMutex);
	if(err != 0)
	{
		return -1;
	}
	jobbuff = pMsgBuf;
	return imsgCode;
}
