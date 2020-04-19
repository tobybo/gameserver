#include <stdlib.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <atomic>
#include <string.h>

#include <vector>
#include <list>
#include <map>
#include <string>

#include <mysql.h>

#include "macro.h"
#include "net_comm.h"
#include "c_slogic.h"
#include "c_crc32.h"
#include "global.h"
#include "ngx_funs.h"
#include "logic_comm.h"
#include "c_dbopt.h"
#include "logic_sql.h"

using std::string;

bool CLogicSocket::onPlyaerRgist( lp_connection_t pConn,
								  LPSTRUC_MSG_HEADER pMsgHeader,
								  char* pPkgBody,
							      unsigned short iBodyLength)
{
	log(LOG,"[LOGIN] onPlyaerRgist start.");
	LPMSGSTR_PT_PLY_REGIST regist_stru = (LPMSGSTR_PT_PLY_REGIST)pPkgBody;
	if(regist_stru->playerAccount == nullptr ||
			regist_stru->playerPwd == nullptr ||
				regist_stru->playerName == nullptr)
	{
		log(ERROR,"[LOGIN] onPlayerRgist error, nullptr");
		return false;
	}

	char* account = trimStr(regist_stru->playerAccount);
	char* pswd    = trimStr(regist_stru->playerPwd);
	char* name    = trimStr(regist_stru->playerName);

	//检查账号是否合法
	if(strlen(account) >= 19 || !checkStr(account,strlen(account)))
	{
		log(ERROR,"[LOGIN] onPlayerRgist error, invalid account: %s",account);
		return false;
	}

	//检查密码是否合法
	if(strlen(pswd) >= 8 || !checkStr(pswd,strlen(pswd)))
	{
		log(ERROR,"[LOGIN] onPlayerRgist error, invalid pswd: %s",pswd);
		return false;
	}

	//检查名字是否合法
	if(strlen(name) >= 20 || !checkStr(name,strlen(name)))
	{
		log(ERROR,"[LOGIN] onPlayerRgist error, invalid name: %s",name);
		return false;
	}

	CDbopt db_conn;
	char sql[100]={0,};
	//char* sql;
	if(sprintf(sql, SQL_CONF[SQL_REGIST_CHECK], account) < 0)
	{
		log(ERROR,"[LOGIN] onPlayerRgist error, invalid sql1, account: %s",account);
		return false;
	}
	MYSQL_RES* res = db_conn.selectOpt(sql);
	MYSQL_ROW column = mysql_fetch_row(res);
	if(std::stoi(column[0]) > 0)
	{
		log(ERROR,"[LOGIN] onPlayerRgist error, had registed, account: %s",account);
		mysql_free_result(res);
		return false;
	}
	if(sprintf(sql, SQL_CONF[SQL_REGIST_ACCOUNT], account, pswd, name) < 0)
	{
		log(ERROR,"[LOGIN] onPlayerRgist error, invalid sql2, account: %s, pswd: %s, name: %s",
				account,pswd,name);
		mysql_free_result(res);
		return false;
	}
	if(!db_conn.updateOpt(sql))
	{
		log(ERROR,"[LOGIN] onPlayerRgist error, excute sql.");
		mysql_free_result(res);
		return false;
	}
	res = db_conn.selectOpt(SQL_CONF[SQL_SELECT_LASTID]);
	column = mysql_fetch_row(res);
	unsigned short pid = std::stoi(column[0]);
	log(LOG,"[LOGIN] onPlayerRgist succ, account: %s, pid: %d",account,pid);

	LPMSGSTR_ST_PLY_REGIST regist_stru_st = new MSGSTR_ST_PLY_REGIST;
	regist_stru_st->playerInfo = new OBJSTRU_PLY_INFO;
	regist_stru_st->playerInfo->playerLevel = 1;
	regist_stru_st->playerId      = pid;
	memcpy(regist_stru_st->playerAccount, account, sizeof(account));
	memcpy(regist_stru_st->playerName, name, sizeof(name));

	msgSend(pConn,ST_ON_PLY_REGIST,(char*)regist_stru_st,(unsigned short)sizeof(MSGSTR_ST_PLY_REGIST));
	delete regist_stru_st; //内存已经被copy到sendbuf
	mysql_free_result(res);
	log(INFO,"[LOGIC_LOGIN] onPlyaerRgist succ, account: %s, name: %s, pswd: %s, pid: %d",
			account, name, pswd, pid);
	return true;
}

bool CLogicSocket::onPlyaerLogin( lp_connection_t pConn,
								  LPSTRUC_MSG_HEADER pMsgHeader,
								  char* pPkgBody,
							      unsigned short iBodyLength)
{
	log(INFO,"[LOGIC_LOGIN] onPlyaerLogin succ, buff: %s",pPkgBody);
	return true;
}
