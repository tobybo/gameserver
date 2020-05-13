#include "c_logic_lua.h"
#include "c_lua_utils.h"
#include "global.h"
#include "ngx_funs.h"
#include "macro.h"
#include "config.h"
#include "c_dbconn.h"
#include "c_player_mng.h"
#include "c_threadpool.h"

#include <pthread.h>
#include <sys/unistd.h>
#include <sys/time.h>

#include <string>
#include <sstream>

#include "LuaIntf.h"

#include "bsoncxx/json.hpp"

int gOn;
int gFrame;
int gTime; //timestamp
unsigned long gMsec; //ms
static const int gFrameTime = 50;  // ms per frame
static const int gFrameCount = 20; // frames per sec

void* luaintf_binding_and_run(void*);

void CLogicLua::init()
{
	m_lua = LuaIntf::LuaState::newState();
	m_lua.openLibs();
	gOn = 1;
	pthread_t pthread_handle;
	int ret = pthread_create(&pthread_handle,nullptr,luaintf_binding_and_run,nullptr);
	if(0 != ret )
	{
		log(ERROR,"[LUAINTF] CLogicLua, pthread_create err, ret: %d",ret);
	}
	ret = pthread_create(&pthread_handle,nullptr,threadLoopTime,nullptr);
	if(0 != ret )
	{
		log(ERROR,"[LUAINTF] CLogicLua, pthread_create err, ret: %d",ret);
	}
}

void* luaintf_binding_and_run(void*)
{
	using namespace LuaIntf;
	LuaState lua = g_logicLua.getLua();
	LuaBinding(lua).beginClass<CLuaUtils>("utils")
		.addConstructor(LUA_ARGS(_opt<std::string>))
		//.addStaticProperty("home_url", &Web::home_url, &Web::set_home_url)
		//.addStaticFunction("go_home", &Web::go_home)
		//.addProperty("url", &Web::url, &Web::set_url)
		//日志
		.addFunction("log", &CLuaUtils::utils_log)
		//读消息
		.addFunction("getMsgCount", &CLuaUtils::getMsgCount)
		.addFunction("getMsgInfo", &CLuaUtils::getMsgInfo)
		.addFunction("readString", &CLuaUtils::readString)
		.addFunction("readInt", &CLuaUtils::readInt)
		.addFunction("readByte", &CLuaUtils::readByte)
		.addFunction("readUInt", &CLuaUtils::readUInt)
		.addFunction("readUByte", &CLuaUtils::readUByte)
		//写消息
		.addFunction("flushSendBuff", &CLuaUtils::flushSendBuff)
		.addFunction("sendMsg", &CLuaUtils::sendMsg)
		.addFunction("writeString", &CLuaUtils::writeString)
		.addFunction("writeInt", &CLuaUtils::writeInt)
		.addFunction("writeByte", &CLuaUtils::writeByte)
		.addFunction("writeUInt", &CLuaUtils::writeUInt)
		.addFunction("writeUByte", &CLuaUtils::writeUByte)
		//mongo
		.addFunction("getDbResCount", &CLuaUtils::getDbResCount)
		.addFunction("flushMongoBuff", &CLuaUtils::flushMongoBuff)
		.addFunction("writeDocument", &CLuaUtils::writeDocument)
		.addFunction("runCommandMongo", &CLuaUtils::runCommandMongo)
		.addFunction("getDbResInfo", &CLuaUtils::getDbResInfo)

		//.addFunction("load", &Web::load, LUA_ARGS(_opt<std::string>))
		//.addStaticFunction("lambda", [] {
				// you can use C++11 lambda expression here too
				//             return "yes";
				//
		//		})
	.endClass();
	bool ret = lua.doFile("/root/work/repos/gameserver/script/main.lua");
	if(ret!=0)
	{
		std::string errstr = g_logicLua.stackDump();
		log(ERROR,errstr.c_str());
	}
	else
	{
		unsigned long curmsec,mHead,mOver;
		struct timeval tv_next;
		long nsleep;
		while(!gTime) sleep(1);
		gFrame = 0;
		while(gOn)
		{
			curmsec = gMsec;
			mHead = curmsec - curmsec % gFrameTime;
			mOver = mHead + gFrameTime;
			if(curmsec - mHead > mOver - curmsec) mOver += gFrameTime;
			gFrame++;
			g_logicLua.doLuaLoop();
			nsleep = mOver - gMsec;
			if(nsleep > 0)
			{
				nsleep = nsleep > gFrameTime? gFrameTime:nsleep;
				tv_next.tv_sec = 0;
				tv_next.tv_usec = nsleep * 1000;
				select(0, nullptr, nullptr, nullptr, &tv_next);
			}
		}
	}
	log(INFO,"[PROC] proc_lua_test end, ret: %d",ret?1:0);
	return (void*)0;
}

std::string CLogicLua::stackDump()
{
	using namespace LuaIntf;
	int stackSize = lua_gettop(m_lua);

	std::string allInfo;
	for (int index = 1;index <= stackSize;index++)
	{
		int t = lua_type(m_lua, index);
		std::string strInfo;
		switch (t)
		{
			case LUA_TSTRING:
				{
					strInfo = lua_tostring(m_lua, index);
					break;

				}
			case LUA_TBOOLEAN:
				{
					strInfo = lua_toboolean(m_lua, index) ? "true" : "false";
					break;

				}
			case LUA_TNUMBER:
				{
					lua_Number result = lua_tonumber(m_lua, index);
					std::stringstream ss;
					ss << result;
					ss >> strInfo;
					break;

				}
			default:
				{
					strInfo = lua_typename(m_lua, index);
					break;

				}

		};
		allInfo = allInfo + strInfo.c_str() + "\n";
		printf("%s\n",strInfo.c_str());

	}
	return allInfo;
}

void* CLogicLua::threadLoopTime(void*)
{
	while(gOn)
	{
		static struct timeval tv_now;
		struct timeval tv_out;
		gettimeofday(&tv_now, nullptr);
		tv_out.tv_sec = 0;
		tv_out.tv_usec = 1000 - tv_now.tv_usec % 1000;
		select(0, nullptr, nullptr, nullptr, &tv_out);
		gettimeofday(&tv_now, nullptr);
		gTime = tv_now.tv_sec;
		gMsec = gTime * 1000 + tv_now.tv_usec * 0.001;
	}
	return (void*)0;
}

void CLogicLua::doLuaLoop()
{
	int msgCount = g_threadpool.m_iRecvMsgQueueCount;
	int dbResCount = getDbResCount();
	//if(msgCount > 0 || dbResCount > 0)
	{
		try{
			//static ...
			m_luaref = LuaIntf::LuaRef(m_lua, "main_loop");
			m_luaref(gTime,gFrame,msgCount,dbResCount);
		}
		catch(LuaIntf::LuaException(m_lua))
		{
			log(ERROR,"[LUA_EXCEPTION] info: %s",LuaIntf::LuaException(m_lua).what());
		}
	}
}

int CLogicLua::getDbResCount()
{
	CMongoConn* db_instance = CMongoConn::GetInstance();
	return db_instance->m_resList.size();
}
