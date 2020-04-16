#include <mysql.h>
#include <string>

#include "macro.h"
#include "global.h"
#include "ngx_funs.h"
#include "c_dbconn.h"
#include "config.h"
#include "c_memory.h"

static LPSTRU_DB_CONN create_db_conns(CDbconn*);

CDbconn* CDbconn::m_instance = nullptr;

CDbconn::CDbconn()
{
	m_free_dbconn = create_db_conns(this);
}

CDbconn::~CDbconn()
{
	CMemory* mem_instance = CMemory::GetInstance();
	auto pos = m_dbconns.begin();
	for(; pos < m_dbconns.end(); pos++)
	{
		if(*pos)
		{
			mysql_close((*pos)->pDbConn);
			mysql_library_end();
			mem_instance->FreeMemory(*pos);
		}
	}
	m_dbconns.clear();
}

static LPSTRU_DB_CONN create_db_conns(CDbconn* db_instance)
{
	CConfig* conf_instance = CConfig::getInstance();
	int conns = std::stoi((*conf_instance)["DB_CONN"]);
	const char* host = (*conf_instance)["DB_HOST"].c_str();
	int port = std::stoi((*conf_instance)["DB_PORT"]);
	const char* pswd = (*conf_instance)["DB_PSWD"].c_str();
	const char* name = (*conf_instance)["DB_NAME"].c_str();
	const char* user = (*conf_instance)["DB_USER"].c_str();

	LPSTRU_DB_CONN pHeader;

	CMemory* mem_instance = CMemory::GetInstance();

	for(int i = 0; i < conns; i++)
	{
		LPSTRU_DB_CONN pConn = (LPSTRU_DB_CONN)mem_instance->AllocMemory(sizeof(STRU_DB_CONN),true);
		MYSQL* db_conn = mysql_init(nullptr);
		if(mysql_real_connect(db_conn,host,user,pswd,name,port,NULL,0))
		{
			log(LOG,"[START] init_db_conns succ, num: %d", i + 1);
			pConn->pDbConn = db_conn;
			pConn->next = pHeader;
			pHeader = pConn;
			db_instance->m_dbconns.push_back(pConn);
		}
		else
		{
			log(ERROR,"[START] init_db_conns err");
			exit(3);
		}
	}
	return pHeader;
}

LPSTRU_DB_CONN CDbconn::getDb()
{
	LPSTRU_DB_CONN pConn = m_free_dbconn;
	if(pConn == nullptr)
	{
		log(ERROR,"[DBWORK] getDb err, all conns busy!");
		return pConn;
	}
	m_free_dbconn = pConn->next;
	return pConn;
}
