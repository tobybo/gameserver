#ifndef _C_CRC32_H_
#define _C_CRC32_H_

#include <stddef.h>

class CCRC32
{
	private:
		CCRC32();
	public:
		~CCRC32(){}
		static CCRC32* GetInstance(){
			if(m_instance == nullptr)
			{
				if(m_instance == nullptr)
				{
					m_instance = new CCRC32();
					static CGarhuishou cl;
				}
			}
			return m_instance;
		}
		class CGarhuishou
		{
		public:
			~CGarhuishou(){
				if(CCRC32::m_instance != nullptr)
				{
					delete CCRC32::m_instance;
					CCRC32::m_instance = nullptr;
				}
			}
		};

	public:
		static CCRC32* m_instance;

	public:
		void Init_CRC32_Table();
		unsigned int Reflect(unsigned int ref, char ch);
		int Get_CRC(unsigned char* buffer, unsigned int dwSize);

	public:
		unsigned int crc32_table[256];
};


#endif
