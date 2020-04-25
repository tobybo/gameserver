//日志相关
#include<cstring>
#include<iostream>
#include<fstream>
#include<stdarg.h>
#include<stdint.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>
#include"macro.h"
#include"config.h"

using std::cout;
using std::endl;

static std::fstream log_fs;

static u_char error_level[][10] = {
	{"[ERROR]"},
	{"[LOG]"},
	{"[INFO]"},
};

//static u_char* log_insert_num(u_char *start,u_char *end,uint64_t val,uintptr_t width);

static u_char* log_insert_num(u_char *start,u_char *end,uint64_t ui64,uintptr_t width){
	u_char *p,temp[MAX_INT64];
	uint32_t ui32;
	p = temp+MAX_INT64-1;

	if(ui64 <= (uint64_t)MAX_UINT32){
		ui32 = (uint32_t)ui64;
		//在32位的cpu上执行64bit计算需要两条指令 而且后一条的执行依赖上一条的执行结果
		do{
			*p-- = (u_char)(ui32 % 10+'0');
		}while(ui32/=10);
	}else{
		do{
			*p-- = (u_char)(ui64 % 10+'0');
		}while(ui64/=10);
	}
	p++;
	size_t len = MAX_INT64 - (p - temp);
	while(len++<width && start < end)
		*start++ = '0';
	len = MAX_INT64 - (p - temp);
	len = len > (end - start)?end-start:len;
	if(len > 0)
		return my_memcpy(start,p,len);
	else
		return start;
}

void log_build_str(u_char* start,u_char* end,const char* fmt,va_list args){
	/*
	 #ifdef _WIN64
	    typedef unsigned __int64  uintptr_t;
	 #else
	    typedef unsigned int uintptr_t;
	 #endif
	 */
	uintptr_t sign,width; //1 signint 0 unsigned int
	u_char *p = nullptr;
	int64_t i64;
	uint64_t ui64;


	while(*fmt && start < end){
		if(*fmt == '%'){
			sign = 1;
			i64 = 0;
			ui64 = 0;
			width = 0;
			fmt++;
			while(*fmt >= '0' && *fmt <= '9' )
			{
				width = width*10+(*fmt++ - '0');
			}
			switch(*fmt){
				case 'u':
					sign = 0;
					fmt++;
				default:
					break;
			}
			switch(*fmt){
				case 'd':
					if(sign){
						i64 = (int64_t)va_arg(args,int);
						if(i64 < 0){
							ui64 = (uint64_t)-i64;
							if(start < end)
								*start++ = '-';
						}else{
							ui64 = (uint64_t)i64;
						}
					}else{
						ui64 = (uint64_t)va_arg(args,u_int);
					}
					//start , end, ui64
					if(start < end)
						start = log_insert_num(start,end,ui64,width);
					break;
				case 's':
					p = va_arg(args,u_char*);
					while(*p && start<end){
						*start++ = *p++;
					}
					break;
				case '%':
					if(start < end)
						*start++ = *fmt;//%%认为保留第二个%
					break;
				default:
					if(start < end)
						*start++ = *(fmt - 1);
			}
			fmt++;
		}
		else{
			*start++ = *fmt++;
		}
	}
	*start = '\0';
}

u_char* log_format_str(size_t len,const char* fmt,...){
	va_list va_args;
	va_start(va_args,fmt);
	u_char* p = new u_char[len+1];
	log_build_str(p,p+len+1,fmt,va_args);
	va_end(va_args);
	return p;
}

void log(u_char level,const char* fmt,...){
	//u_char *buff = new u_char[1025];
	static u_char buff[1025];
	u_char *start;
	u_char *end = buff+1024; //预留一个位置可以放下'\0'
	size_t len_node = strlen((const char *)error_level[level]);
	start = my_memcpy(buff,error_level[level],len_node);

	struct timeval tv; //linux 自带的库
	struct tm tmv;      //c 标准库

	memset(&tv,0,sizeof(struct timeval));
	memset(&tmv,0,sizeof(struct tm));

	gettimeofday(&tv,nullptr);
	time_t sec = tv.tv_sec;
	localtime_r(&sec,&tmv);

	tmv.tm_mon++;
	tmv.tm_year += 1900;
	//[2020-03-22 00:14:00]
	u_char * time_str = log_format_str(22,"[%4d-%2d-%2d %2d:%2d:%2d]",
			tmv.tm_year,tmv.tm_mon,tmv.tm_mday,
			tmv.tm_hour,tmv.tm_min,tmv.tm_sec);
	start = my_memcpy(start,time_str,21);

	va_list va_args;
	va_start(va_args,fmt);
	log_build_str(start,end,fmt,va_args);
	va_end(va_args);

    /*c++98 void open (const   char* filename,  ios_base::openmode mode = ios_base::in);*/
    /*c++11特性 void open (const string& filename,  ios_base::openmode mode = ios_base::in);*/

	log_done(buff);
}

void log_done(const u_char* buff)
{
	log_fs<<buff<<endl;

	CConfig* config_instance = CConfig::getInstance();
	if(std::stoi((*config_instance)["DAEMON"]) == 0)
	{
		std::cout<<buff<<std::endl;
	}
}

void log_done(std::string& buff)
{
	log_fs<<buff.c_str()<<endl;

	CConfig* config_instance = CConfig::getInstance();
	if(std::stoi((*config_instance)["DAEMON"]) == 0)
	{
		std::cout<<buff.c_str()<<std::endl;
	}
}

void log_lua_test(std::string &str)
{
	log_done(str);
}

bool log_open_file(const std::string& filename){
    log_fs.open(filename,std::fstream::app);
	return (bool)log_fs;
}

void log_close_file(){
	if(log_fs)
		log_fs.close();
}

void log_record_master_pid()
{
	pid_t pid = getpid();
	std::fstream pid_fs;
    pid_fs.open("gmpid.log",std::fstream::in|std::fstream::out|std::fstream::trunc);
	pid_fs<<pid;
	if(pid_fs)
		pid_fs.close();
}

/*unix 环境高级编程 条件变量*/
void maketimeout(struct timespec* tsp, long seconds)
{
	struct timeval now;

	gettimeofday(&now, nullptr);
	tsp->tv_sec = now.tv_sec;
	tsp->tv_nsec = now.tv_usec * 1000;

	tsp->tv_sec += seconds;

}
