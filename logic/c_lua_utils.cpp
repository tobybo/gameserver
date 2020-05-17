#include <iostream>
#include <string>
#include <string_view>
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
#include "c_mongo_conn.h"
#include "c_timer.h"

#include "LuaIntf.h"
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/stdx/string_view.hpp>

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
	m_joblen = 0;
	m_maxlen = _PKG_MAX_LENGTH - 1000;
	m_sendbuff = new char[m_maxlen];
	m_sendlen = 0;
	memset(m_sendbuff, 0, m_maxlen);

	m_msg_info = nullptr;
	m_db_res = nullptr;
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

int CLuaUtils::getDbResCount()
{
	return g_logicLua.getDbResCount();
}

std::tuple<unsigned short, int, uint64_t> CLuaUtils::getMsgInfo()
{
	freeJobbuff();
	unsigned short imsgCode(0);
	int idx(0);
	uint64_t icq(0);
	int ret = g_socket.getJobBuff(m_jobbuff, m_jobpos, m_joblen);
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
	m_joblen = 0;
}

void CLuaUtils::freeMongobuff()
{
	if(m_msg_info)
	{
		delete m_msg_info;
		m_msg_info = nullptr;
	}
}

bool CLuaUtils::isOverFlow(int diff)
{
	//m_jobpos + diff <= m_joblen
	return m_joblen < m_jobpos + diff;
}

std::string CLuaUtils::readString()
{
	if(isOverFlow(1))return "";
	char* tmp = m_jobbuff + m_jobpos;
	m_jobpos += strlen(tmp) + 1;
	if(isOverFlow(0))return "";
	string data(tmp);
	return data;
}

int CLuaUtils::readInt()
{
	if(isOverFlow(sizeof(int)))return 0;
	int data = ntohl(*((int*)(m_jobbuff + m_jobpos)));
	m_jobpos += sizeof(int);
	if(isOverFlow(1))return 0;
	return data;
}

char CLuaUtils::readByte()
{
	if(isOverFlow(sizeof(char)))return 0;
	return *(m_jobbuff + m_jobpos++);
}

u_int CLuaUtils::readUInt()
{
	if(isOverFlow(sizeof(u_int)))return 0;
	u_int data = ntohl(*((u_int*)(m_jobbuff + m_jobpos)));
	m_jobpos += sizeof(u_int);
	return data;
}

u_char CLuaUtils::readUByte()
{
	if(isOverFlow(sizeof(u_char)))return 0;
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

void CLuaUtils::flushMongoBuff(int _requestId,int _dbNum,int _opMode,int _noCallBack,std::string _collName,std::string _sqlStr)
{
	freeMongobuff();
	m_msg_info = new STRU_DB_ASKMSG;
	m_msg_info->requestId  = _requestId;
	m_msg_info->dbNum      = _dbNum;
	m_msg_info->opMode     = _opMode;
	m_msg_info->noCallBack = _noCallBack;
	m_msg_info->collName   = _collName;
	m_msg_info->sqlStr     = _sqlStr;
}

void CLuaUtils::runCommandMongo()
{
	CMongoConn* mongo_instatnce = CMongoConn::GetInstance();
	LPSTRU_DB_ASKMSG msg_item = new STRU_DB_ASKMSG;
	//memcpy(msg_item,m_msg_info,sizeof(STRU_DB_ASKMSG));
	*msg_item = *m_msg_info;
	int err = pthread_mutex_lock(&(mongo_instatnce->m_mutex_query));
	if(err != 0)
		log(ERROR,"[LUA_UTILS] runCommandMongo, lock query err, err: %d",err);
	mongo_instatnce->m_queryList.push_back(msg_item);
	err = pthread_mutex_unlock(&(mongo_instatnce->m_mutex_query));
	if(err != 0)
		log(ERROR,"[LUA_UTILS] runCommandMongo, unlock query err, err: %d",err);
	err = pthread_cond_signal(&(mongo_instatnce->m_cond_query));
	if(err != 0)
		log(ERROR,"[LUA_UTILS] runCommandMongo, signal query err, err: %d",err);
}

void CLuaUtils::writeDocument(std::string _doc)
{
	//log(INFO,"======== %s",_doc.c_str());
	m_msg_info->doc = _doc;

	//--------------------------------------
	//CMongoConn* db_conn = CMongoConn::GetInstance();
	//(db_conn->m_db_game)["test1"].insert_one(m_msg_info->doc);
	//mongocxx::client conn{mongocxx::uri{}};
	//bsoncxx::builder::stream::document document{};
	//document<<"name"<<"toby";
	//auto collection = conn["game1"]["test1"];
	//collection.insert_one(document.view());     yes
	//collection.insert_one(*(m_msg_info->doc));  no
	//collection.insert_one(bsoncxx::from_json(stv).view()); yes
	//collection.insert_one(view); no

	//---------------------------------------
	//for(auto& e:_tab)
	//{
		//LuaIntf::LuaRef key = e.key<>();
		//m_msg_info->doc<<key;
		//LuaIntf::LuaRef val = e.value<>();
		//m_msg_info->doc<<val;
	//}

	//bsoncxx::document::value a = bsoncxx::from_json(_doc);
	//log(INFO,"========2 %s",bsoncxx::to_json(a.view()));
	//---------------------------------------
}

std::tuple<int,int,int,std::string> CLuaUtils::getDbResInfo()
{
	int ret(0);
	int requestId(0);
	int opMode(0);
	std::string res("");
	CMongoConn* db_instance = CMongoConn::GetInstance();
	db_instance->getOutResList(m_db_res);
	if(m_db_res)
	{
		ret = m_db_res->ret;
		requestId = m_db_res->requestId;
		opMode = m_db_res->opMode;
		res = m_db_res->resStr;
		delete m_db_res;
		m_db_res = nullptr;
	}
	auto tp = std::make_tuple(ret,requestId,opMode,res);
	return tp;
}

void CLuaUtils::addTimer(u_long cd_msec, int timer_id, int int_param1, int int_param2, std::string string_param1, std::string string_param2)
{
	CTimer* timer_instance = CTimer::GetInstance();
	timer_instance->addTimer(cd_msec,timer_id,int_param1,int_param2,string_param1,string_param2);
}

int CLuaUtils::getTimerCount()
{
	CTimer* timer_instance = CTimer::GetInstance();
	return timer_instance->getTimerCount();
}

std::tuple<int,int,int,std::string,std::string> CLuaUtils::getTimerInfo()
{
	CTimer* timer_instance = CTimer::GetInstance();
	int timerId(0);
	int p1(0);
	int p2(0);
	std::string p3 = "";
	std::string p4 = "";
	lpstr_events event = timer_instance->getOneEvent();
	if(event)
	{
		timerId = event->eventId;
		p1      = event->int_param1;
		p2      = event->int_param2;
		p3      = event->string_param1;
		p4      = event->string_param2;

		delete event;
		event = nullptr;
	}
	auto ret = std::make_tuple(timerId,p1,p2,p3,p4);
	return ret;
}
