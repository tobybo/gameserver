#ifndef _NGX_FUNS_H_
#define _NGX_FUNS_H_

#include <string>

typedef struct{
	std::string name;
	std::string content;
}SConfInfo,*SConfInfoPtr;

void testDefFun();
char* trimStrLeft(char* s,int len,const char c);
void trimStrRight(char* s,int len,const char c);
char* trimStr(char* s, const char c);
char* trimStr(char* s);
bool checkStr(char* s, int len);

void title_move_environ();
void title_set(const char* title);

void log(u_char level,const char* fmt,...);
bool log_open_file(const std::string& filename);
void log_close_file();
void log_record_master_pid();
void maketimeout(struct timespec* tsp, long seconds);

int proc_set_daemon();
int proc_create_childs();

int init_signals();

#endif


