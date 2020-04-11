#include "c_slogic.h"

CLogicSocket::CLogicSocket(){

}

CLogicSocket::~CLogicSocket(){

}

bool CLogicSocket::Initialize(){
	bool bParentInit = CSocket::Initialize();
	return bParentInit;
}

//处理具体消息
void threadRecvProcFunc(char *pMsgBuf){

	return;
}
