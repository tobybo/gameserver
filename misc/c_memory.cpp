#include <strings.h>

#include "c_memory.h"

CMemory::*m_instance = NULL;

void *AllocMemory(int memCount, bool ifmemset){
	void* tmpData = (void*)new char[memCount];
	if(ifmemset){
		bzero(tempData,memCount);
	}
	return tmpData;
}

void CMemory::FreeMemory(void *point)
{
	delete []((char *)point);
}
