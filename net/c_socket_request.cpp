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
#include "net_comm.h"

using std::string;
using std::cout;
using std::endl;

void CSocket::read_request_handler(lp_connection_t pConn)
{
	ssize_t n = recvproc(pConn,pConn->precvbuf,pConn->irecvlen);
	if(n <= 0)
		return;
	switch(pConn->curStat)
	{
		case _PKG_HD_INIT:
			if(n == pConn->irecvlen)
			{
				wait_request_handler_proc_p1(pConn);
			}
			else
			{
				//包头没有接收完
				pConn->curStat   = _PKG_HD_RECVING;
				pConn->precvbuf += n;
				pConn->irecvlen  = pConn->irecvlen - n;
			}
			break;
		case _PKG_HD_RECVING:
			if(n == pConn->irecvlen)
			{
				wait_request_handler_proc_p1(pConn);
			}
			else
			{
				//包头没有接收完
				//pConn->curStat   = _PKG_HD_RECVING;
				pConn->precvbuf += n;
				pConn->irecvlen  = pConn->irecvlen - n;
			}
			break;
		case _PKG_BD_INIT:
			if(n == pConn->irecvlen)
			{
				wait_request_handler_proc_last(pConn);
			}
			else
			{
				//包体没有接收完
				pConn->curStat   = _PKG_BD_RECVING;
				pConn->precvbuf += n;
				pConn->irecvlen  = pConn->irecvlen - n;
			}
			break;
		case _PKG_BD_RECVING:
			if(n == pConn->irecvlen)
			{
				wait_request_handler_proc_last(pConn);
			}
			else
			{
				//包体仍然没有接收完
			    //pConn->curStat   = _PKG_BD_RECVING;
				pConn->precvbuf += n;
				pConn->irecvlen  = pConn->irecvlen - n;
			}
			break;
		default:
			break;
	}
	//cout<<"[SOCKET] read_request_handler: "<<buff<<endl;
	log(INFO,"[SOCKET] read_request_handler runing, fd: %d, recvstat: %d, recvsize: %d",pConn->fd,pConn->curStat,n);
	return;
}

ssize_t CSocket::recvproc(lp_connection_t pConn,char* buff,ssize_t bufflen)
{
	ssize_t n;
	n = recv(pConn->fd,buff,bufflen,0);
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
		return -1;
	}
	if(n<0)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK)
		{
			log(ERROR,"[RECVPKG] recvproc err, errno: %d",errno);
			return -1;
		}
		if(errno == EINTR)
		{
			log(ERROR,"[RECVPKG] recvproc err, errno: %d",errno);
			return -1;
		}
		if(errno == ECONNRESET)
		{
			//客户端粗暴关闭 不四路握手而是直接发rst包
		}
		log(ERROR,"[RECVPKG] recvproc err and close fd, errno: %d, fd: %d",errno,pConn->fd);
		if(close(pConn->fd) == -1)
		{
			log(ERROR,"[RECVPKG] recvproc close fd err, fd: %d",pConn->fd);
		}
		inRecyConnectQueue(pConn);
	}
	return n;
}

void CSocket::wait_request_handler_proc_p1(lp_connection_t pConn)
{
	CMemory* mem_instance = CMemory::GetInstance();

	LPCOMM_PKG_HEADER pPkgHeader;
	pPkgHeader = (LPCOMM_PKG_HEADER)pConn->dataHeadInfo;

	unsigned short e_pkgLen;
	e_pkgLen = ntohs(pPkgHeader->pkgLen);

	if(e_pkgLen < m_iLenPkgHeader)
	{
		//包长比包头还小
		pConn->curStat = _PKG_HD_INIT;
		pConn->precvbuf = pConn->dataHeadInfo;
		pConn->irecvlen = m_iLenPkgHeader;
	}
	else if(e_pkgLen > (_PKG_MAX_LENGTH - 1000))
	{
		//包太长了 丢弃
		pConn->curStat = _PKG_HD_INIT;
		pConn->precvbuf = pConn->dataHeadInfo;
		pConn->irecvlen = m_iLenPkgHeader;
	}
	else
	{
		char* temp = (char*)mem_instance->AllocMemory(e_pkgLen+m_iLenMsgHeader,true);
		pConn->precvMemPointer = temp;

		//添加消息头
		LPSTRUC_MSG_HEADER pMsgHeader = (LPSTRUC_MSG_HEADER)temp;
		pMsgHeader->pConn = pConn;
		pMsgHeader->iCurrsequence = pConn->iCurrsequence;
		temp += m_iLenMsgHeader;

		//添加包头
		memcpy(temp,pPkgHeader,m_iLenPkgHeader);
		/*LPCOMM_PKG_HEADER _pPkgHeader = (LPCOMM_PKG_HEADER)temp;*/
		//_pPkgHeader->pkgLen  = e_pkgLen;
		//_pPkgHeader->msgCode = ntohs(pPkgHeader->msgCode);
		/*_pPkgHeader->crc32   = ntohl(pPkgHeader->crc32);*/

		if(e_pkgLen == m_iLenPkgHeader)
		{
			//包长度 == 包头长度 无包体
			wait_request_handler_proc_last(pConn);
		}
		else
		{
			//准备接收包体
			pConn->curStat  = _PKG_BD_INIT;
			pConn->precvbuf = temp + m_iLenPkgHeader;
			pConn->irecvlen = e_pkgLen - m_iLenPkgHeader;
		}
	}
}

void CSocket::wait_request_handler_proc_last(lp_connection_t pConn)
{
	//收到完整包 可以加入消息队列 唤醒逻辑线程处理具体业务
	log(INFO,"[RECVPKG] wait_request_handler_proc_last succ, buff: %s",pConn->precvMemPointer);
	g_threadpool.inMsgRecvQueueAndSignal(pConn->precvMemPointer);
	pConn->precvMemPointer = nullptr;
	pConn->curStat = _PKG_HD_INIT;
	pConn->precvbuf = pConn->dataHeadInfo;
	pConn->irecvlen = m_iLenPkgHeader;
}

void CSocket::write_request_handler(lp_connection_t pConn)
{
	char test_str[] = "hi,i'm a linux server.";
	send(pConn->fd,(void*)test_str,sizeof(test_str),0);
	//cout<<"[SOCKET] write_request_handler~ "<<endl;
	log(INFO,"[SOCKET] write_request_handler~");
}
