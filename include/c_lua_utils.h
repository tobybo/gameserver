/*给lua脚本用的工具类*/
#ifndef _C_LUA_UTILS_H_
#define _C_LUA_UTILS_H_

#include <string>
#include <tuple>

#include "net_comm.h"
#include "macro.h"
#include "c_mongo_conn.h"

#include <LuaIntf.h>

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

	public: //mongo相关
		int getDbResCount();
		void flushMongoBuff(int _requestId,int _dbNum,int _opMode,int _noCallBack,std::string _collName,std::string _sqlStr);
		void runCommandMongo();
		void writeDocument(std::string _js);
		std::tuple<int,int,int,std::string> getDbResInfo();

	public: //定时器相关
		void addTimer(u_long cd_msec, int timer_id, int int_param1, int int_param2, std::string string_param1, std::string string_param2);
		int getTimerCount();
		std::tuple<int,int,int,std::string,std::string> getTimerInfo();

	private:
		void freeJobbuff();
		void freeMongobuff();
		bool isOverFlow(int);

	private: //收发消息
		char* m_jobbuff;
		int m_jobpos;
		int m_joblen;
		char* m_sendbuff;
		int m_sendlen;
		static unsigned short m_maxlen;

	private: //mongo相关
		LPSTRU_DB_ASKMSG m_msg_info; //临时仓库 放入请求队列时拷贝
		LPSTRU_DB_ASKRES m_db_res;   //从返回数据队列拉过来 用完清理
};

#endif
