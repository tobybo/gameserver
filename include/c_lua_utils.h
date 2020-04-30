/*给lua脚本用的工具类*/
#ifndef _C_LUA_UTILS_H_
#define _C_LUA_UTILS_H_

#include <string>
#include <tuple>

#include "net_comm.h"
#include "macro.h"

class CLuaUtils
{
	private:
		CLuaUtils(); //luaintf 不支持默认构造函数 也可能是我没发现怎么用 后面在研究
	public:
		CLuaUtils(std::string&); //lua使用的这个构造函数 不用传参数 luaintf默认给个空串
		~CLuaUtils();

	public: //日志
		void utils_log(std::string _buff);

	public: //接收消息
		int getMsgCount();
		std::tuple<unsigned short, int, uint64_t> getMsgInfo();

		std::string readString();
		int readInt();
		char readByte();

		u_int readUInt();
		u_char readUByte();

	public: //发送消息
		void flushSendBuff();
		void sendMsg(unsigned short _imsgCode, int _idx, uint64_t _icq);

		void writeString(std::string&);
		void writeInt(int);
		void writeByte(char);

		void writeUInt(u_int);
		void writeUByte(u_char);

	private:
		void freeJobbuff();

	private:
		char* m_jobbuff;
		int m_jobpos;
		char* m_sendbuff;
		int m_sendlen;
		static unsigned short m_maxlen;
};

#endif
