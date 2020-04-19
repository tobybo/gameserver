#ifndef _C_PLAYER_T_H
#define _C_PLAYER_T_H

#include <string>

#include "c_socket.h"

using std::string;

class CPlayer_t
{
	private:
		CPlayer_t();
	public:
		explicit CPlayer_t(unsigned int _pid);
		~CPlayer_t();

	public:
		unsigned int getId(){ return m_id; }
		string getAccount(){ return m_account; }
		void setAccount(string _account){ m_account = _account; }
		string getName(){ return m_name; }
		void setName(string _name){ m_name = _name; }
		unsigned short getLevel(){ return m_level; }
		void setLevel(unsigned short _level){ m_level = _level; }

	public:
		char state; //0 online 1 offline
		lp_connection_t pConn;
		time_t offLineTime;
	private:
		string m_account;
		string m_name;
		unsigned int m_id;
		unsigned short int m_level;
};

#endif
