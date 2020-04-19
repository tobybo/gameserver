#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <map>

#include <signal.h>
#include <c_threadpool.h>
#include <c_slogic.h>

extern char **g_os_arg;
extern char *gp_envmem;
extern int gp_environlen;

extern int g_isdaemon;
extern int proc_id;
extern int parent_id;
extern int proc_type;
extern sig_atomic_t proc_reap;

extern CLogicSocket g_socket;
extern CThreadPool g_threadpool;

extern int g_stopEvent;

extern std::map<unsigned short,const char*> SQL_CONF;

#endif
