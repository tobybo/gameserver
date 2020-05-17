#ifndef _C_SLOGIC_H_
#define _C_SLOGIC_H_

#include <map>
#include "c_socket.h"

class CLogicSocket : public CSocket
{
public:
	CLogicSocket();
	virtual ~CLogicSocket();
	virtual bool Initialize();

public:
	//逻辑层收发消息通用方法
	void msgSend(lp_connection_t pConn, unsigned short msgCode, char* pPkgBody,unsigned short bodyLen);

public:
	//业务逻辑具体消息处理函数
	bool onPlyaerRgist( lp_connection_t pConn,
					    LPSTRUC_MSG_HEADER pMsgHeader,
						char* pPkgBody,
						unsigned short iBodyLength);
	bool onPlyaerLogin( lp_connection_t pConn,
					    LPSTRUC_MSG_HEADER pMsgHeader,
						char* pPkgBody,
						unsigned short iBodyLength);

public:
	virtual void threadRecvProcFunc(char *pMsgBuf);
public:
	int getJobBuff(char*& jobbuff, int& jobpos, int& joblen);
	/*void addConn(LPSTRUC_MSG_HEADER pMsgHeader);*/
	//void addConn(lp_connection_t pConn);
	//void delConn(LPSTRUC_MSG_HEADER pMsgHeader); //自己释放pMsgHeader
	//void delConn(lp_connection_t pConn);
	/*void delConn(int sockid);*/

private:
	std::map<int,LPSTRUC_MSG_HEADER> m_socketConnMap;
};

#endif
