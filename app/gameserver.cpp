#include<unistd.h>
#include<signal.h>
#include<iostream>
#include"config.h"
#include"ngx_funs.h"
#include"macro.h"
#include"c_socket.h"
#include"c_slogic.h"
#include"c_threadpool.h"
#include"c_memory.h"
#include"c_crc32.h"

char **g_os_arg; //系统参数的全局指针
char *gp_envmem = nullptr; //指向新分配的系统环境变量的内存
int gp_environlen = 0; //环境变量所占内存大小

int g_isdaemon;
int proc_id;
int parent_id;
int proc_type;
sig_atomic_t proc_reap;

CLogicSocket g_socket;
CThreadPool g_threadpool;
int g_stopEvent;

int main(int argc,char* const* argv){

    g_os_arg = (char **)argv; //为什么阔以强制转换呢

    int exitcode = 0;
	g_stopEvent = 0;
    proc_reap = 0;
    CConfig* config_instance = CConfig::getInstance();
    using std::cout;
    using std::endl;
    //testDefFun();
    //1 加载配置
    if(!config_instance->loadConfig("server.conf"))
	{
        cout<<"load_conf faild"<<endl;
		exitcode = 2;
		goto lblexit;
	}
    else
    {
        cout<<"load_conf succ"<<endl;
        //cout<<"port: "<<(*config_instance)["PORT"]<<endl;
    }

	CMemory::GetInstance();
	CCRC32::GetInstance();

    //2 给环境参数搬家
    title_move_environ();

    //3 打开日志文件
    if(!log_open_file((*config_instance)["LOG"]))
    {
        cout<<"打开日志文件失败，请检查配合路径是否存在!!"<<endl;
        exitcode = -1;
        goto lblexit;
    }
    /*log(ERROR,"this is a log, level is: %d,it is mean: %s",ERROR,"error");*/
    //log(LOG,"this is a log, level is: %d,it is mean: %s",LOG,"log");
    //log(INFO,"this is a log, level is: %d,it is mean: %s",INFO,"info");
    //4 初始化 信号，监听socket
    if(init_signals()!=0){
        exitcode = 1;
        goto lblexit;
    }
    if(g_socket.Initialize()==false)
    {
        exitcode = 1;
        goto lblexit;
    }

    //5 创建守护进程
    if(std::stoi((*config_instance)["DAEMON"]) == 1)
    {
        int ret = proc_set_daemon();
        if(ret == -1)
        {
            //创建失败退出
            exitcode = -1;
            goto lblexit;
        }
        if(ret == 1){
            //父进程退出
            exitcode = 1;
            goto lblexit;
        }
        g_isdaemon = 1; //守护进程模式
        log(LOG,"[SERVER] start server, set daemon succ");
    }

    //6 创建子进程
    proc_create_childs();

    /*for(;;){*/
        //sleep(5);
        //cout<<"after sleep 5 s."<<endl;
    /*}*/
lblexit:
    log(LOG,"[PROC] init proc come, pid: %d",getpid());
    log_close_file();
    return exitcode;
}
