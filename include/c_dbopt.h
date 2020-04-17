#ifndef _C_DBOPT_
#define _C_DBOPT_

#include <mysql.h>

#include "c_dbconn.h"

class CDbopt
{
	public:
		CDbopt(){
			CDbconn* db_instance = CDbconn::GetInstance();
			m_conn = db_instance->getDb();
		}
		~CDbopt(){
			if(m_conn)
			{
				CDbconn* db_instance = CDbconn::GetInstance();
				db_instance->freeDb(m_conn);
			}
		}

	public:
		bool updateOpt(const char*); //数据改变的操作
		MYSQL_RES* selectOpt(const char*); //查询数据


	private:
		LPSTRU_DB_CONN m_conn;
};

#endif
