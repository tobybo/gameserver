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
	log(INFO,"[RECVPKG] wait_request_handler_proc_last succ");
	g_threadpool.inMsgRecvQueueAndSignal(pConn->precvMemPointer);
	pConn->precvMemPointer = nullptr;
	pConn->curStat = _PKG_HD_INIT;
	pConn->precvbuf = pConn->dataHeadInfo;
	pConn->irecvlen = m_iLenPkgHeader;
}

void CSocket::write_request_handler(lp_connection_t pConn)
{
	CMemory* mem_instance = CMemory::GetInstance();

	ssize_t n = sendproc(pConn,pConn->psendbuf,pConn->isendlen);
	if(n > 0 && n != pConn->isendlen)
	{
		//发送正常 但是没有发送完整 继续等待 LT模式的epoll下次触发
		pConn->psendbuf += n;
		pConn->isendlen -= n;
		//这里本来就是 epoll写事件触发的回调 不用再 ++pConn->iThrowsendCount;
		return;
	}
	else if(n == -1)
	{
		//epoll通知我可写 可是缓冲区居然是满的
		log(ERROR,"[MSG_SEND] write_request_handler err1");
		return;
	}

	if(n >= pConn->isendlen)
	{
		//发送完成 需要把写事件从epoll红黑树里移除
		if(epoll_oper_event(pConn->fd,
					EPOLL_CTL_MOD,
					EPOLLOUT,
					1, //0 增加 1 去掉 2 覆盖
					pConn) == -1)
		{
			log(ERROR,"[MSG_SEND] write_request_handler epoll_mod err");
		}
		log(INFO,"[MSG_SEND] write_request_handler send all succ");
	}
	else
	{
		log(INFO,"[SOCKET] write_request_handler send err2");
		//不做特殊处理 这种情况认为对端断开 在read事件会干掉连接和从epoll红黑树里移除
	}

	//要么发送完 要么对端断开 做内存释放和发送队列清理
	mem_instance->FreeMemory(pConn->psendMemPointer);
	pConn->psendMemPointer = NULL;
	--pConn->iThrowsendCount;
	if(sem_post(&m_semEventSendQueue) == -1)
		log(ERROR,"[MSG_SEND] write_request_handler sem_post err");
	return;
}

ssize_t CSocket::sendproc(lp_connection_t pConn,char* buff,ssize_t bufflen)
{
	ssize_t n;
	for(;;)
	{
		n = send(pConn->fd,buff,bufflen,0);
		if(n > 0)
		{
			//发送成功n字节
			return n;
		}

		if(n == 0)
		{
			//超时 连接断开epoll会通知到rhander处理
			return 0;
		}

		if(errno == EAGAIN)
		{
			//内核缓冲区满
			return -1;
		}

		if(errno == EINTR)
		{
			log(LOG,"[MSG_SEND] sendproc send errno is EINTR");
		}
		else
		{
			return -2;
		}
	}
}
