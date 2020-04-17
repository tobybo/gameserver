#ifndef _LOGIC_COMM_H
#define _LOGIC_COMM_H

//协议定义
#define PT_ON_PLY_REGIST 0x1000 //玩家注册
#define PT_ON_PLY_LOGIN  0x1001 //玩家登陆
#define ST_ON_PLY_REGIST 0x1002 //玩家注册反馈
#define ST_ON_PLY_LOGIN  0x1003 //玩家登陆反馈


//协议数据结构定义 消息的包体结构
#pragma pack(1)

typedef struct
{
	char playerAccount[20];
	char playerPwd[8];
	char playerName[20];

}MSGSTR_PT_PLY_REGIST,*LPMSGSTR_PT_PLY_REGIST;

typedef struct _STRU_PLY_INFO
{
	unsigned short playerLevel;
}OBJSTRU_PLY_INFO,*LPOBJSTRU_PLY_INFO;

typedef struct
{
	unsigned int playerId;
	char playerAccount[20];
	char playerName[20];
	LPOBJSTRU_PLY_INFO playerInfo;

}MSGSTR_ST_PLY_REGIST,*LPMSGSTR_ST_PLY_REGIST;

#pragma pack()

#endif
