#include <mysql.h>

#include "macro.h"
#include "global.h"
#include "ngx_funs.h"
#include "c_dbopt.h"


bool CDbopt::updateOpt(const char* _sql)
{
	if(!m_conn)
	{
		log(ERROR,"[DBWORK] updateOpt err conn, sql: %s", _sql);
		return false;
	}
	int ret = mysql_query(m_conn->pDbConn, _sql);
	if(ret == 0)
	{
		log(LOG,"[DBWORK] updateOpt succ, sql: %s", _sql);
		return true;
	}
	else
	{
		log(LOG,"[DBWORK] updateOpt err, errstr: %s, sql: %s", mysql_error(m_conn->pDbConn), _sql);
		return false;
	}
}

MYSQL_RES* CDbopt::selectOpt(const char* _sql)
{
	MYSQL_RES* res;
	if(!m_conn)
	{
		log(ERROR,"[DBWORK] selectOpt err conn, sql: %s", _sql);
		return res;
	}
	int ret = mysql_query(m_conn->pDbConn, _sql);
	if(ret == 0)
	{
		log(LOG,"[DBWORK] selectOpt succ, sql: %s", _sql);
	}
	else
	{
		log(LOG,"[DBWORK] selectOpt err, errstr: %s, sql: %s", mysql_error(m_conn->pDbConn), _sql);
		return res;
	}

	res = mysql_store_result(m_conn->pDbConn);
	return res;
}
