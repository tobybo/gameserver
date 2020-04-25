#include "c_logic_lua.h"
#include "c_lua_utils.h"
#include "global.h"
#include "ngx_funs.h"
#include "macro.h"
#include "config.h"
#include "c_dbconn.h"
#include "c_player_mng.h"

#include <pthread.h>

#include <string>
#include <sstream>

#include "LuaIntf.h"

void* luaintf_binding_and_run(void*);

void CLogicLua::init()
{
	m_lua = LuaIntf::LuaState::newState();
	m_lua.openLibs();
	pthread_t pthread_handle;
	int ret = pthread_create(&pthread_handle,nullptr,luaintf_binding_and_run,nullptr);
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
		.addFunction("log", &CLuaUtils::utils_log)
		.addFunction("getMsgCount", &CLuaUtils::getMsgCount)
		.addFunction("getMsgInfo", &CLuaUtils::getMsgInfo)
		.addFunction("readString", &CLuaUtils::readString)
		.addFunction("readInt", &CLuaUtils::readInt)
		.addFunction("readByte", &CLuaUtils::readByte)
		.addFunction("readUInt", &CLuaUtils::readUInt)
		.addFunction("readUByte", &CLuaUtils::readUByte)
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

