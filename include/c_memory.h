#ifndef _C_MEMORY_H_
#define _C_MEMORY_H_

class CMemory
{
private:
	CMemory(){}
public:
	~CMemory(){}
	static CMemory* GetInstance(){
		if(m_instance == NULL){
			//锁
			if(m_instance == NULL){
				m_instance = new CMemory();
			}
		}
		return m_instance;
	}
	class CGarhuishou
	{
	public:
		~CGarhuishou(){
			if(CMemory::m_instance){
				delete CMemory::m_instance;
				CMemory::m_instance = NULL;
			}
		}
	}

public:
	void *AllocMemory(int memCount, bool ifmemset);
	void FreeMemory(void *point);

private:
	static CMemory* m_instance;
}

#endif
