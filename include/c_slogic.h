#ifndef _C_SLOGIC_H_
#define _C_SLOGIC_H_

#include "c_socket.h"

class CLogicSocket : public CSocket
{
public:
	CLogicSocket();
	virtual ~CLogicSocket();
	virtual bool Initialize();

public:
	//业务逻辑具体消息处理函数

};

#endif
