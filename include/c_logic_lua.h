#ifndef _C_LOGIC_LUA_H_
#define _C_LOGIC_LUA_H_

#include <string>

#include "LuaIntf.h"

class CLogicLua
{
	public:
		CLogicLua(){}
		~CLogicLua(){}

	public:
		void init();
		LuaIntf::LuaState& getLua(){return m_lua;};

	public:
		std::string stackDump();
		static void* threadLoopTime(void*);
		void doLuaLoop();

	private:
		LuaIntf::LuaState m_lua;
		LuaIntf::LuaRef m_luaref;
};

#endif
