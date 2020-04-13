#ifndef _NET_COMM_H
#define _NET_COMM_H

//这里定义收发数据包的一些格式和状态信息
#define _PKG_MAX_LENGTH 30000 //最大包长  包头+包体 29000  预留一定空间

#define _PKG_HD_INIT    0 //准备接收包头
#define _PKG_HD_RECVING 1 //继续接收包头
#define _PKG_BD_INIT    2 //准备接收包体
#define _PKG_BD_RECVING 3 //继续接收包体

#define _DATA_BUFSIZE_ 20 //包头缓冲区长度

#pragma pack(1)

typedef struct
{
	unsigned short pkgLen;
	unsigned short msgCode;

	int            crc32;
}COMM_PKG_HEADER,*LPCOMM_PKG_HEADER;

#pragma pack()

#endif
