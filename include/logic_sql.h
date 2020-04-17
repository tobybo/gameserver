#ifndef _LOGIC_SQL_
#define _LOGIC_SQL_

#include <map>

#define SQL_SELECT_LASTID     0x0101 //获取当前线程自增id

#define SQL_REGIST_CHECK      0x1000 //检查玩家是否已经注册过
#define SQL_REGIST_ACCOUNT    0x1001 //执行玩家注册


std::map<unsigned short,const char*> SQL_CONF = {
	{ SQL_SELECT_LASTID     ,    "select last_insert_id();" },

	{ SQL_REGIST_CHECK      ,    "select count(*) from PLAYER_TBL where account = '%s';" },
	{ SQL_REGIST_ACCOUNT    ,    "insert into PLAYER_TBL (account, pswd, name) values ('%s', '%s', '%s');" },
};


#endif
