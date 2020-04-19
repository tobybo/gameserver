#ifndef _C_DBCONN_H
#define _C_DBCONN_H

#include <stdlib.h>
#include <atomic>
#include <vector>

#include <mysql.h>
#include <pthread.h>

typedef struct _STRU_DB_CONN
{
	MYSQL* pDbConn;
	struct _STRU_DB_CONN* next;
}STRU_DB_CONN,*LPSTRU_DB_CONN;

class CDbconn
{
private:
	CDbconn();
public:
	~CDbconn();
	static CDbconn* GetInstance(){
		if(m_instance == NULL)
		{
			m_instance = new CDbconn();
			static GarHuiShou clr;
		}
		return m_instance;
	}
	class GarHuiShou{
		public:
			~GarHuiShou(){
				if(CDbconn::m_instance)
				{
					delete CDbconn::m_instance;
					CDbconn::m_instance = nullptr;
				}
			}
	};

public:
	LPSTRU_DB_CONN getDb();
	void freeDb(LPSTRU_DB_CONN);

public:
	LPSTRU_DB_CONN m_free_dbconn;
	std::vector<LPSTRU_DB_CONN> m_dbconns;

private:
	pthread_mutex_t m_thread_mutex_db_conn;

private:
	static CDbconn* m_instance;
};

#endif
