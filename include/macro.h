#ifndef _MACRO_H_
#define _MACRO_H_

#define MAX_UINT32 (uint32_t) 0xffffffff
#define MAX_INT64 (sizeof("-9223372036854775808")-1)

#define ERROR 0
#define LOG   1
#define INFO  2

#define MASTER_PROC 0 //守护进程
#define WORKER_PROC 1 //工作进程

#define my_memcpy(dst,src,n) ( ((u_char*)memcpy(dst,src,n)) + (n))

#endif
