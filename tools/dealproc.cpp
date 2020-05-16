//处理进程

#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>
#include<string.h>

#include<string>
#include<iostream>

#include"global.h"
#include"ngx_funs.h"
#include"macro.h"
#include"config.h"
#include"c_dbconn.h"
#include"c_player_mng.h"
#include"c_logic_lua.h"
#include"c_timer.h"

#include "LuaIntf.h"

using std::cout;
using std::endl;

void proc_lua_test();

int proc_set_daemon(){
	switch(fork()){
		case -1:
			return -1;
		case 0:
			break;
		default:
			return 1;
	}
	parent_id = proc_id;
	proc_id = getpid();
	if(setsid() == -1)
	{
		log(ERROR,"server start faild, setsid err");
		return -1;
	}
	umask(0);
	int fd = open("/dev/null",O_RDWR);
	if(fd == -1)
	{
		log(ERROR,"server start faild, open fd err");
		return -1;
	}
	if(dup2(fd,STDIN_FILENO) == -1){
		log(ERROR,"server start faild, set STDIN fd err");
		return -1;
	}
	if(dup2(fd,STDOUT_FILENO) == -1){
		log(ERROR,"server start faild, set STDOUT fd err");
		return -1;
	}
	if(fd > STDERR_FILENO){
		if(close(fd) == -1){
			log(ERROR,"server start faild, close fd err");
			return -1;
		}
	}
	return 0;
}

void proc_child_init(){
	//清空信号集合 接收所有信号
	sigset_t set;
	sigemptyset(&set);
	if(sigprocmask(SIG_SETMASK,&set,NULL) == -1)
	{
		log(ERROR,"[PROC] proc_child_init setmask err");
	}

	//建立监听套接字
	if(g_socket.open_listening_sockets()==false)
    {
		exit(-4);
    }

	//初始化数据库连接
	CDbconn::GetInstance();

	//初始化玩家管理器
	CPlayerMng::GetInstance();

	//初始化定时器管理器
	CTimer::GetInstance();

	//初始化线程池 用lua取消息 这里多线程没意义了
	/*CConfig *config_instance = CConfig::getInstance();*/
	//int thread_count = stoi((*config_instance)["LOGIC_THREAD_COUNT"]);
	//if(!g_threadpool.Create(thread_count)){
		//exit(-2);
	/*}*/
	//sleep(1);
	//socket init sub
	if(g_socket.Initialize_subporc() == false){
		exit(-2);
	}

	g_socket.epoll_process_init();

	return;
}

void proc_lua_init()
{
	g_logicLua.init();
}

int proc_child_circle(int num){
	string proc_name = "cb3_worker_"+std::to_string(num)+" ";
	title_set(proc_name.c_str());
	log(LOG,"[PROC] worker_proc begin working,num: %d, pid: %d",num,getpid());

	proc_child_init();
	proc_lua_init();

	for(;;){
		//cout<<"child proc: "<<proc_name<<endl;
		g_socket.epoll_process_events(-1);
	}

	CPlayerMng* ply_mng = CPlayerMng::GetInstance();
	ply_mng->stopThread();

	g_threadpool.StopAll();
	g_socket.Shutdown_subproc();

	return 0;
}

int proc_create_childs(){
	sigset_t set;
	sigemptyset(&set);

	sigaddset(&set, SIGCHLD);     //子进程状态改变
	sigaddset(&set, SIGALRM);     //定时器超时
	sigaddset(&set, SIGIO);       //异步I/O
	sigaddset(&set, SIGINT);      //终端中断符
	sigaddset(&set, SIGHUP);      //连接断开
	sigaddset(&set, SIGUSR1);     //用户定义信号
	sigaddset(&set, SIGUSR2);     //用户定义信号
	sigaddset(&set, SIGWINCH);    //终端窗口大小改变
	sigaddset(&set, SIGTERM);     //终止
	sigaddset(&set, SIGQUIT);     //终端退出符
	//.........可以根据开发的实际需要往其中添加其他要屏蔽的信号......

	if(sigprocmask(SIG_BLOCK,&set,NULL)==-1)
		log(LOG,"[SIGNAL] proc_create_childs, sigprocmask err");

    CConfig* config_instance = CConfig::getInstance();
	for(int i = 0;i<stoi((*config_instance)["PROCS"]);i++)
	{
		pid_t pid;
		pid = fork();
		log(LOG,"[PROC] proc_create_child, fork pid: %d",pid);
		switch(pid){
			case -1:
				log(ERROR,"start server failed, create child err");
				goto lblexit;
			case 0:
				parent_id = proc_id;
				proc_id = getpid();
				proc_child_circle(i);
				break;
			default:
				log(INFO,"[SIGNAL] proc_create_childs succ, num: %d, pid: %d",i,pid);
				break; //主进程跳过继续创建下一个
		}
	}

	sigemptyset(&set);
	//设置master进程的名字
    title_set("cb3_master ");
	log_record_master_pid();
	for(;;){
		log(LOG,"[PROC] master_proc is working, pid: %d",getpid());
		sigsuspend(&set);
		sleep(1);
	}
lblexit:
	log(ERROR,"[PROC] create_child_proc err!");
	return -1;
}

