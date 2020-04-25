#include <string>
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
	int err = pthread_mutex_lock(&g_threadpool.m_pthreadMutex);
	if(err != 0)
	{
		return -1;
	}
	return g_threadpool.m_iRecvMsgQueueCount;
}

unsigned short CLuaUtils::getMsgInfo()
{
	freeJobbuff();
	int imsgCode = g_socket.getJobBuff(m_jobbuff, m_jobpos);
	if(imsgCode <= 0)
	{
		m_jobbuff = nullptr;
		m_jobpos = 0;
		return imsgCode;
	}
	return imsgCode;
}

void CLuaUtils::freeJobbuff()
{
	if(m_jobbuff)
	{
		CMemory* mem_instance = CMemory::GetInstance();
		mem_instance->FreeMemory(m_jobbuff);
		m_jobbuff = nullptr;
		m_jobpos = 0;
	}
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
