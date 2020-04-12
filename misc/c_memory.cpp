#include <strings.h>
#include <stddef.h>

#include "c_memory.h"

CMemory* CMemory::m_instance = NULL;

void *CMemory::AllocMemory(int memCount, bool ifmemset){
	void* tmpData = (void*)new char[memCount];
	if(ifmemset){
		bzero(tmpData,memCount);
	}
	return tmpData;
}

void CMemory::FreeMemory(void *point)
{
	delete []((char *)point);
}
