#include <string>
#include <tuple>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include "global.h"
#include "c_threadpool.h"
#include "c_memory.h"
#include "macro.h"
#include "c_crc32.h"
#include "c_lua_utils.h"
#include "ngx_funs.h"
#include "config.h"
#include "c_dbconn.h"
#include "c_player_mng.h"
#include "net_comm.h"

unsigned short CLuaUtils::m_maxlen = _PKG_MAX_LENGTH - 1000;

CLuaUtils::CLuaUtils()
{
	m_jobbuff = nullptr;
	m_jobpos = 0;
	m_sendbuff = new char[m_maxlen];
	m_sendlen = 0;
	memset(m_sendbuff, 0, m_maxlen);
	//log(INFO,"[LUA_UTILS] CLuaUtils");
}

CLuaUtils::CLuaUtils(std::string&)
{
	m_jobbuff = nullptr;
	m_jobpos = 0;
	m_maxlen = _PKG_MAX_LENGTH - 1000;
	m_sendbuff = new char[m_maxlen];
	m_sendlen = 0;
	memset(m_sendbuff, 0, m_maxlen);
	//log(INFO,"[LUA_UTILS] CLuaUtils string");
}

CLuaUtils::~CLuaUtils()
{
	freeJobbuff();
}

void CLuaUtils::utils_log(std::string _buff)
{
	log_lua_test(_buff);
}

int CLuaUtils::getMsgCount()
{
	if(g_threadpool.m_iRecvMsgQueueCount <= 0)
		return 0;
	return g_threadpool.m_iRecvMsgQueueCount;
}

std::tuple<unsigned short, int, uint64_t> CLuaUtils::getMsgInfo()
{
	freeJobbuff();
	unsigned short imsgCode(0);
	int idx(0);
	uint64_t icq(0);
	int ret = g_socket.getJobBuff(m_jobbuff, m_jobpos);
	if(ret <= 0)
	{
		freeJobbuff();
	}
	else
	{
		imsgCode = (unsigned short)ret;
		LPSTRUC_MSG_HEADER pMsgHeader = (LPSTRUC_MSG_HEADER)m_jobbuff;
		icq = pMsgHeader->iCurrsequence;
		idx = pMsgHeader->pConn->vIndex;
	}
	auto tp = std::make_tuple(imsgCode,idx,icq);
	return tp;
}

void CLuaUtils::freeJobbuff()
{
	if(m_jobbuff)
	{
		CMemory* mem_instance = CMemory::GetInstance();
		mem_instance->FreeMemory(m_jobbuff);
		m_jobbuff = nullptr;
	}
	m_jobpos = 0;
}

std::string CLuaUtils::readString()
{
	char* tmp = m_jobbuff + m_jobpos;
	string data(tmp);
	m_jobpos += strlen(tmp) + 1;
	return data;
}

int CLuaUtils::readInt()
{
	int data = ntohl(*((int*)(m_jobbuff + m_jobpos)));
	m_jobpos += sizeof(int);
	return data;
}

char CLuaUtils::readByte()
{
	return *(m_jobbuff + m_jobpos++);
}

u_int CLuaUtils::readUInt()
{
	u_int data = ntohl(*((u_int*)(m_jobbuff + m_jobpos)));
	m_jobpos += sizeof(u_int);
	return data;
}

u_char CLuaUtils::readUByte()
{
	return (u_char)(*(m_jobbuff + m_jobpos++));
}

void CLuaUtils::flushSendBuff()
{
	m_sendlen = 0;
}

void CLuaUtils::sendMsg(unsigned short _imsgCode, int _idx, uint64_t _icq)
{

	lp_connection_t pConn = g_socket.get_used_connection(_idx);
	if(!pConn) return;
	if(pConn->iCurrsequence != _icq)
	{
		log(ERROR,"[LUA_UTILS] sendMsg, it is a old connmsg, _imsgCode: %d",_imsgCode);
		return;
	}
	g_socket.msgSend(pConn,_imsgCode,m_sendbuff,m_sendlen);
}

void CLuaUtils::writeString(std::string& _str)
{
	int str_len = _str.size() + 1; //放到发送缓冲区 需要0来分隔不同的字符串
	if(str_len + m_sendlen > m_maxlen)
	{
		log(ERROR,"[LUA_UTILS] writeString, errlen, now: %d, addstr: %s, addlen: %d",m_sendlen,_str,str_len);
		return;
	}
	strcpy(m_sendbuff + m_sendlen, _str.c_str()); //including the terminating null character
	m_sendlen += str_len;
}

void CLuaUtils::writeInt(int _data)
{
	char data_len = sizeof(int);
	if(data_len + m_sendlen > m_maxlen)
	{
		log(ERROR,"[LUA_UTILS] writeInt, errlen, now: %d",m_sendlen);
		return;
	}
	_data = htonl(_data);
	memcpy(m_sendbuff + m_sendlen, &_data, data_len);
	m_sendlen += data_len;
}

void CLuaUtils::writeByte(char _data)
{
	char data_len = sizeof(char);
	if(data_len + m_sendlen > m_maxlen)
	{
		log(ERROR,"[LUA_UTILS] writeByte, errlen, now: %d",m_sendlen);
		return;
	}
	memcpy(m_sendbuff + m_sendlen, &_data, data_len);
	m_sendlen += data_len;
}

void CLuaUtils::writeUInt(u_int _data)
{
	char data_len = sizeof(u_int);
	if(data_len + m_sendlen > m_maxlen)
	{
		log(ERROR,"[LUA_UTILS] writeUInt, errlen, now: %d",m_sendlen);
		return;
	}
	memcpy(m_sendbuff + m_sendlen, &_data, data_len);
	m_sendlen += data_len;
}

void CLuaUtils::writeUByte(u_char _data)
{
	char data_len = sizeof(u_char);
	if(data_len + m_sendlen > m_maxlen)
	{
		log(ERROR,"[LUA_UTILS] writeUByte, errlen, now: %d",m_sendlen);
		return;
	}
	memcpy(m_sendbuff + m_sendlen, &_data, data_len);
	m_sendlen += data_len;
}

