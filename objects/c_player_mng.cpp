#include <time.h>
#include <errno.h>
#include <map>
#include <list>

#include <pthread.h>

#include "macro.h"
#include "ngx_funs.h"
#include "global.h"
#include "c_player_mng.h"

CPlayerMng* CPlayerMng::m_instance = nullptr;
bool CPlayerMng::m_shutdown = false;
pthread_mutex_t CPlayerMng::m_mutexOffLinePlayerQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  CPlayerMng::m_condOffLinePlayerQueue  = PTHREAD_COND_INITIALIZER;
std::atomic<int> CPlayerMng::m_readOffLineCount;
std::atomic<int> CPlayerMng::m_playerMapCount;
map<unsigned int,CPlayer_t*> CPlayerMng::m_playerMap;
std::list<CPlayer_t*> CPlayerMng::m_readyOffLineQueue;

CPlayerMng::CPlayerMng()
{
	m_readOffLineCount = 0;
	//log(INFO,"[PLAYER_MNG] threadDelPlayerFun: %d",threadDelPlayerFun);
	//log(INFO,"[PLAYER_MNG] &CPlayerMng::threadDelPlayerFun: %d",&CPlayerMng::threadDelPlayerFun);
	int err = pthread_create(&m_pthread_handle, nullptr, threadDelPlayerFun, nullptr);
	if(err != 0)
	{
		log(ERROR,"[PLAYER_MNG] CPlayerMng, pthread_create err, err: %d",err);
	}
}

CPlayerMng::~CPlayerMng()
{
	pthread_mutex_destroy(&m_mutexOffLinePlayerQueue);
	pthread_cond_destroy(&m_condOffLinePlayerQueue);
}

bool CPlayerMng::addPlayer(CPlayer_t* _player)
{
	if(m_playerMap[_player->getId()])
	{
		return false;
	}
	m_playerMap[_player->getId()] = _player;
	++m_playerMapCount;
	return true;
}

CPlayer_t* CPlayerMng::addPlayer(unsigned int _pid)
{
	if(m_playerMap[_pid])
	{
		return m_playerMap[_pid];
	}
	CPlayer_t* _player = new CPlayer_t(_pid);
	addPlayer(_player);
	return _player;
}

CPlayer_t* CPlayerMng::getPlayer(unsigned int _pid)
{
	return m_playerMap[_pid];
}

void CPlayerMng::outOffLinePlayer(unsigned int _pid)
{
	auto iter = m_readyOffLineQueue.begin();
	for(; iter != m_readyOffLineQueue.end(); iter++)
	{
		if((*iter)->getId() == _pid)
		{
			(*iter)->state = 0;
			m_readyOffLineQueue.erase(iter);
			--m_readOffLineCount;
			break;
		}
	}
}

void CPlayerMng::afterPlayerLogin(unsigned int _pid,
					  string _account,
					  string _name)
{
	CPlayer_t* _player = addPlayer(_pid);
	if(_player->state == 1)
	{
		outOffLinePlayer(_pid);
	}
	_player->setAccount(_account);
	_player->setName(_name);
}

bool CPlayerMng::offLinePlayer(unsigned int _pid)
{
	CPlayer_t* _player = getPlayer(_pid);
	return offLinePlayer(_player);
}

bool CPlayerMng::offLinePlayer(CPlayer_t* _player)
{
	if(_player == nullptr || _player->state == 1)
		return false;
	_player->state = 1;
	time_t now = time(nullptr);
	_player->offLineTime = now;
	m_readyOffLineQueue.push_back(_player);
	++m_readOffLineCount;

	return true;
}

std::map<unsigned int,CPlayer_t*> CPlayerMng::getOnlinePlayers()
{
	return m_playerMap;
}

void* CPlayerMng::threadDelPlayerFun(void *)
{
	timespec tsp;
	while(!m_shutdown)
	{
		maketimeout(&tsp, 60);
		int err = pthread_cond_timedwait(&m_condOffLinePlayerQueue, &m_mutexOffLinePlayerQueue, &tsp);
		if(err != 0 && err != ETIMEDOUT)
			log(ERROR,"[PLAYER_MNG] threadDelPlayerFun, mutex_lock err, err: %d",err);
		if(m_readOffLineCount <= 0)
		{
			err = pthread_mutex_unlock(&m_mutexOffLinePlayerQueue);
			if(err != 0)
				log(ERROR,"[PLAYER_MNG] threadDelPlayerFun, mutex_unlock 1 err");
			continue;
		}
		if(m_readyOffLineQueue.empty())
		{
			log(ERROR,"[PLAYER_MNG] threadDelPlayerFun, count err, count: %d",(int)m_readOffLineCount);
			return (void*)0;
		}
		time_t now = time(nullptr);
		auto iter_begin = m_readyOffLineQueue.begin();
		auto iter_end = m_readyOffLineQueue.end();
		auto iter = iter_begin;
		int count = 0;
		for(; iter != iter_end; iter++)
		{
			if((*iter)->offLineTime + 60 > now)
			{
				break;
			}
			count++;
		}
		for(int i = 0; i < count; i++)
		{
			//real offline
			m_playerMap.erase(m_readyOffLineQueue.front()->getId());
			m_readyOffLineQueue.pop_front();
			--m_readOffLineCount;
			--m_playerMapCount;
		}
		err = pthread_mutex_unlock(&m_mutexOffLinePlayerQueue);
		if(err != 0)
			log(ERROR,"[PLAYER_MNG] threadDelPlayerFun, mutex_unlock 2 err");
	}
	return (void*)0;
}

void CPlayerMng::stopThread()
{
	m_shutdown = true;
	int err = pthread_cond_signal(&m_condOffLinePlayerQueue);
	if(err != 0)
		log(ERROR,"[PLAYER_MNG] stopThread, signal err");
	void* tret;
	log(INFO,"[PLAYER_MNG] stopThread, begin join");
	pthread_join(m_pthread_handle, &tret);
	log(INFO,"[PLAYER_MNG] stopThread, end join");
	if(!m_readyOffLineQueue.empty())
	{
		auto iter = m_readyOffLineQueue.begin();
		for(; iter != m_readyOffLineQueue.end(); iter++)
		{
			m_playerMap.erase(m_readyOffLineQueue.front()->getId());
			--m_readOffLineCount;
		}
	}
	m_readyOffLineQueue.clear();
	m_playerMap.clear();
}
