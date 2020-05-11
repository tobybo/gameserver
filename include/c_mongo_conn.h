#ifndef _C_MONGO_CONN_H
#define _C_MONGO_CONN_H

#include<iostream>
#include<bsoncxx/builder/stream/document.hpp>
#include<bsoncxx/json.hpp>
#include<bsoncxx/stdx/string_view.hpp>
#include<mongocxx/client.hpp>
#include<mongocxx/instance.hpp>
#include<mongocxx/database.hpp>
#include<mongocxx/collection.hpp>

#include "macro.h"

#include <stdlib.h>
#include <string>
#include <list>

#include <pthread.h>

typedef struct
{
	int  requestId; //find lua coroutine
	int  dbNum;		//0 gamedb
	int  opMode;	//0 insert 1 find
	int  noCallBack;//1 dont into reslist
	std::string collName;
	std::string doc;
	//bsoncxx::stdx::string_view doc;
	//bsoncxx::document::view* doc;
	//bsdocument docStream;
	std::string sqlStr;
}STRU_DB_ASKMSG,*LPSTRU_DB_ASKMSG;

typedef struct
{
	int ret; //0 succ 1 failed
	int requestId;
	int opMode;
	//查询结果
	std::string resStr;
	//mongocxx::cursor curs;
}STRU_DB_ASKRES,*LPSTRU_DB_ASKRES;

class CMongoConn
{
private:
	CMongoConn();
public:
	~CMongoConn();
	static CMongoConn* GetInstance(){
		if(m_instance == NULL)
		{
			m_instance = new CMongoConn();
			static GarHuiShou clr;
		}
		return m_instance;
	};
	class GarHuiShou{
		public:
			~GarHuiShou(){
				if(CMongoConn::m_instance)
				{
					delete CMongoConn::m_instance;
					CMongoConn::m_instance = nullptr;
				}
			}
	};

public:
	static void* threadFunAskDb(void*);
	void runCommand(LPSTRU_DB_ASKMSG _msg_info);
	void pushIntoResList(LPSTRU_DB_ASKRES _res_item);
	void getOutResList(LPSTRU_DB_ASKRES& _res_item);

private:
	static CMongoConn* m_instance;

public:
	mongocxx::instance  m_inst;
	mongocxx::client    m_conn;
	mongocxx::database  m_db_game;
	mongocxx::database  m_db_global;

	std::list<LPSTRU_DB_ASKMSG> m_queryList;
	std::list<LPSTRU_DB_ASKRES> m_resList;

	pthread_mutex_t m_mutex_query;
	pthread_mutex_t m_mutex_res;

	pthread_cond_t m_cond_query;
};

#endif
