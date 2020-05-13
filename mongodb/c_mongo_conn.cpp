#include<iostream>
#include<bsoncxx/builder/stream/document.hpp>
#include<bsoncxx/stdx/string_view.hpp>
#include<bsoncxx/json.hpp>
#include<mongocxx/client.hpp>
#include<mongocxx/instance.hpp>
#include<mongocxx/database.hpp>
#include<mongocxx/collection.hpp>

#include <stdlib.h>
#include <string>
#include <list>

#include <pthread.h>

#include "macro.h"
#include "ngx_funs.h"
#include "config.h"
#include "c_mongo_conn.h"
#include "global.h"

CMongoConn* CMongoConn::m_instance = nullptr;

CMongoConn::CMongoConn()
{
	CConfig* cfg_instance = CConfig::getInstance();
	std::string host = (*cfg_instance)["MDB_HOST"];
	int port = std::stoi((*cfg_instance)["MDB_PORT"]);
	char* uri = new char[100]{0,};
	sprintf(uri,"mongodb://%s/%d",host.c_str(),port);
	log(INFO,"[MONGO] constructor, uri: %s",uri);
	m_conn = mongocxx::client(mongocxx::uri(uri));
	delete []uri;
	if(!m_conn)
	{
		log(ERROR,"[MONGO] CMongoConn, connect err, uri: %s",uri);
		exit(-3);
	}
	std::string game_name = (*cfg_instance)["MDB_GAME_NAME"];
	std::string global_name = (*cfg_instance)["MDB_GLOBAL_NAME"];
	m_db_game = m_conn[game_name];
	m_db_global = m_conn[global_name];
	if(!m_db_game || !m_db_global)
	{
		log(ERROR,"[MONGO] CMongoConn, find db err, uri: %s",uri);
		exit(-3);
	}
	m_mutex_query = PTHREAD_MUTEX_INITIALIZER;
	m_mutex_res   = PTHREAD_MUTEX_INITIALIZER;
	m_cond_query  = PTHREAD_COND_INITIALIZER;
	pthread_t thread_handle;
	int err = pthread_create(&thread_handle,NULL,threadFunAskDb,NULL);
	if(err!=0)
	{
		log(ERROR,"[MONGO] CMongoConn, create err, err: %d",err);
		exit(-4);
	}
}

CMongoConn::~CMongoConn()
{

}

void* CMongoConn::threadFunAskDb(void* _threadData)
{
	int err;
	while(true)
	{
		err = pthread_mutex_lock(&(m_instance->m_mutex_query));
		if(err != 0)
			log(ERROR,"[MONGO] threadFunAskDb, lock query err, err: %d",err);
		if(m_instance->m_queryList.size() == 0 && gOn)
		{
			pthread_cond_wait(&(m_instance->m_cond_query),&(m_instance->m_mutex_query));
		}
		if(!gOn)
		{
			pthread_mutex_unlock(&(m_instance->m_mutex_query));
			break;
		}
		/*1 从前往后遍历 执行数据库操作*/
		/*2 将操作结果加锁放入 res列表*/
		/*3 清空队列*/
		auto iter = m_instance->m_queryList.begin();
		for(;iter != m_instance->m_queryList.end();iter++)
		{
			m_instance->runCommand(*iter);
			delete *iter;
		}
		m_instance->m_queryList.clear();
		err = pthread_mutex_unlock(&(m_instance->m_mutex_query));
		if(err != 0)
			log(ERROR,"[MONGO] threadFunAskDb, unlock query err, err: %d",err);
	}
	return (void*)0;
}

void CMongoConn::runCommand(LPSTRU_DB_ASKMSG _msg_info)
{
	mongocxx::database db;
	if(_msg_info->dbNum == 0)
		db = m_db_game;
	else if(_msg_info->dbNum == 1)
		db = m_db_global;
	else
	{
		log(ERROR,"[MONGO] runCommand, err dbNum, num: %d",_msg_info->dbNum);
		return;
	}
	if(db)
	{
		LPSTRU_DB_ASKRES resItem = new STRU_DB_ASKRES;
		resItem->requestId = _msg_info->requestId;
		resItem->opMode = _msg_info->opMode;
		switch(_msg_info->opMode)
		{
			case 0:
				//insert collection one document
				{
					//auto result = db[_msg_info->collName].insert_one(_msg_info->docStream.view());
					bsoncxx::stdx::string_view stv{_msg_info->doc.c_str(),_msg_info->doc.size()};
					auto result = db[_msg_info->collName].insert_one(bsoncxx::from_json(stv).view());
					if(result)
						resItem->ret = 0;
					else
						resItem->ret = 1;
					if(_msg_info->noCallBack == 0)
						pushIntoResList(resItem);
					break;
				}
			case 1:
				//find document from collection
				{
					//auto cursor = db[_msg_info->collName].find(_msg_info->docStream.view());
					bsoncxx::stdx::string_view stv{_msg_info->doc.c_str(),_msg_info->doc.size()};
					auto cursor = db[_msg_info->collName].find(bsoncxx::from_json(stv).view());
					resItem->resStr = "";
					for (auto&& doc : cursor) {
						resItem->resStr += bsoncxx::to_json(doc);
						resItem->resStr += "|";
					}
					log(INFO,"[MONGO] runCommand, res_str: %s",resItem->resStr.c_str());
					pushIntoResList(resItem);
					break;
				}
			case 2:
				{
					bsoncxx::stdx::string_view filter{_msg_info->doc.c_str(),_msg_info->doc.size()};
					bsoncxx::stdx::string_view update_doc{_msg_info->sqlStr.c_str(),_msg_info->sqlStr.size()};
					auto result = db[_msg_info->collName].update_many(bsoncxx::from_json(filter).view(),bsoncxx::from_json(update_doc).view());
					if(result)
						resItem->ret = 0;
					else
						resItem->ret = 1;
					if(_msg_info->noCallBack == 0)
						pushIntoResList(resItem);
					break;
				}
			default:
				break;
		}
	}
}

void CMongoConn::pushIntoResList(LPSTRU_DB_ASKRES _res_item)
{
	int err = pthread_mutex_lock(&m_mutex_res);
	if(err != 0)
		log(ERROR,"[MONGO] pushIntoResList, lock res err, err: %d",err);
	m_resList.push_back(_res_item);
	err = pthread_mutex_unlock(&m_mutex_res);
	if(err != 0)
		log(ERROR,"[MONGO] pushIntoResList, unlock res err, err: %d",err);
}

void CMongoConn::getOutResList(LPSTRU_DB_ASKRES& _res_item)
{
	if(m_resList.size() <= 0)
		return;
	int err = pthread_mutex_lock(&m_mutex_res);
	if(err != 0)
		log(ERROR,"[MONGO] getOutResList, lock res err, err: %d",err);
	_res_item = m_resList.front();
	m_resList.pop_front();
	err = pthread_mutex_unlock(&m_mutex_res);
	if(err != 0)
		log(ERROR,"[MONGO] getOutResList, unlock res err, err: %d",err);
}
