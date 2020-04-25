/*给lua脚本用的工具类*/
#ifndef _C_LUA_UTILS_H_
#define _C_LUA_UTILS_H_

#include <string>

#include "net_comm.h"
#include "macro.h"

class CLuaUtils
{
	public:
		CLuaUtils(){
			m_jobbuff = nullptr;
			m_jobpos = 0;
		};
		CLuaUtils(std::string&){};
		~CLuaUtils();

	public: //日志
		void utils_log(std::string _buff);
	public: //消息
		int getMsgCount();
		unsigned short getMsgInfo();

		std::string readString();
		int readInt();
		char readByte();

		u_int readUInt();
		u_char readUByte();

	private:
		void freeJobbuff();

	private:
		char* m_jobbuff;
		int m_jobpos;
};

#endif
