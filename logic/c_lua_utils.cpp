#include <string>
#include <tuple>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

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

CLuaUtils::CLuaUtils()
{
	m_jobbuff = nullptr;
	m_jobpos = 0;
	//log(INFO,"[LUA_UTILS] CLuaUtils");
}

CLuaUtils::CLuaUtils(std::string&)
{
	m_jobbuff = nullptr;
	m_jobpos = 0;
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
	int socketid(0);
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
		socketid = pMsgHeader->pConn->fd;
	}
	auto tp = std::make_tuple(imsgCode,socketid,icq);
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
	int data = (int)(*(m_jobbuff + m_jobpos));
	m_jobpos += sizeof(int);
	return data;
}

char CLuaUtils::readByte()
{
	return *(m_jobbuff + m_jobpos++);
}

u_int CLuaUtils::readUInt()
{
	u_int data = (u_int)(*(m_jobbuff + m_jobpos));
	m_jobpos += sizeof(u_int);
	return data;
}

u_char CLuaUtils::readUByte()
{
	return (u_char)(*(m_jobbuff + m_jobpos++));
}
