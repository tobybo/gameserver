#ifndef _C_PLAYER_MNG_
#define _C_PLAYER_MNG_

#include <map>
#include <list>

#include <atomic>
#include <pthread.h>

#include "c_player_t.h"

using std::map;
using std::string;

class CPlayerMng
{
	private:
		CPlayerMng();
	public:
		~CPlayerMng();
		static CPlayerMng* GetInstance(){
			if(m_instance == nullptr)
			{
				m_instance = new CPlayerMng();
				static GarHuiShou cli;
			}
			return m_instance;
		}
		class GarHuiShou
		{
			public:
				~GarHuiShou(){
					if(CPlayerMng::m_instance)
					{
						delete CPlayerMng::m_instance;
						CPlayerMng::m_instance = nullptr;
					}
				}
		};

	private:
		//内部处理
		static void* threadDelPlayerFun(void*);

	public:
		void stopThread();

	public:
		//外部接口
		bool addPlayer(CPlayer_t*);
		CPlayer_t* addPlayer(unsigned int _pid);
		bool offLinePlayer(unsigned int _pid);
		bool offLinePlayer(CPlayer_t*);
		void outOffLinePlayer(unsigned int _pid);
		CPlayer_t* getPlayer(unsigned int _pid);
		map<unsigned int,CPlayer_t*> getOnlinePlayers();
		void afterPlayerLogin(unsigned int _pid,
							  string _account,
							  string _name);

	private:
		static map<unsigned int,CPlayer_t*> m_playerMap;
		std::atomic<int> m_playerMapCount;
		static std::list<CPlayer_t*> m_readyOffLineQueue;
		static std::atomic<int> m_readOffLineCount;
		pthread_t m_pthread_handle;
		static bool m_shutdown;
		static pthread_mutex_t m_mutexOffLinePlayerQueue;
		static pthread_cond_t m_condOffLinePlayerQueue;

	private:
		static CPlayerMng* m_instance;
};

#endif
