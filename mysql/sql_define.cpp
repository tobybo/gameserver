#include <map>
#include "logic_sql.h"

std::map<unsigned short,const char*> SQL_CONF = {
	{ SQL_SELECT_LASTID     ,    "select last_insert_id();" },

	{ SQL_REGIST_CHECK      ,    "select count(*) from PLAYER_TBL where account = '%s';" },
	{ SQL_REGIST_ACCOUNT    ,    "insert into PLAYER_TBL (account, pswd, name) values ('%s', md5('%s'), '%s');" },
};
