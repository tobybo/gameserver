#ifndef _MACRO_H_
#define _MACRO_H_

#include<bsoncxx/builder/stream/document.hpp>

#define MAX_UINT32 (uint32_t) 0xffffffff
#define MAX_INT64 (sizeof("-9223372036854775808")-1)

#define u_int  unsigned int
#define u_long unsigned long

#define ERROR 0
#define LOG   1
#define INFO  2

#define MASTER_PROC 0 //守护进程
#define WORKER_PROC 1 //工作进程

#define my_memcpy(dst,src,n) ( ((u_char*)memcpy(dst,src,n)) + (n))

typedef bsoncxx::builder::stream::document bsdocument;

const int gFrameTime = 50;  // ms per frame
const int gFrameCount = 20;

#endif
