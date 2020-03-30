#include "c_slogic.h"

CLogicSocket::CLogicSocket(){

}

CLogicSocket::~CLogicSocket(){

}

bool CLogicSocket::Initialize(){
	bool bParentInit = CSocket::Initialize();
	return bParentInit;
}
