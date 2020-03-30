#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <strings.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "macro.h"
#include "ngx_funs.h"

typedef struct
{
	int        signo;
	const char *signame;

	void (*handler)(int signo, siginfo_t* siginfo, void *ucontext);

}my_signal_t;

static void my_signal_handler(int signo, siginfo_t* siginfo, void *ucontext);
static void my_process_get_status(void);

my_signal_t signals[] = {
	//signo     signame				 handler
	{ SIGHUP,   "SIGHUP",			 my_signal_handler },
	{ SIGINT,   "SIGINT",			 my_signal_handler },
	{ SIGTERM,  "SIGTERM",			 my_signal_handler },
	{ SIGCHLD,  "SIGCHLD",			 my_signal_handler },
	{ SIGQUIT,  "SIGQUIT",			 my_signal_handler },
	{ SIGIO,    "SIGIO",			 my_signal_handler },
	{ SIGSYS,   "SIGSYS, SIG_IGN",   NULL},

	{ 0,        NULL,       NULL              },
};

int init_signals(){
	my_signal_t *sig;
	struct sigaction sa; //sigaction 有同名函数 所以用 struct来区分

	for(sig = signals; sig->signo != 0; sig++)
	{
		bzero(&sa,sizeof(struct sigaction));
		if(sig->handler){
			sa.sa_sigaction = sig->handler;
			sa.sa_flags = SA_SIGINFO;
		}
		else{
			sa.sa_handler = SIG_IGN;
		}
		sigemptyset(&sa.sa_mask);
		if(sigaction(sig->signo, &sa, NULL) == -1){
			log(ERROR,"[SIGNAL] init_signals err, sigaction failed, signo: %d",sig->signo);
			return -1;
		}
		else{
			log(LOG,"[SIGNAL] init_signals succ, signo: %d",sig->signo);
		}
	}
	return 0;
}

static void my_signal_handler(int signo, siginfo_t* siginfo, void *ucontext )
{
	my_signal_t *sig;
	char *action;

	for(sig = signals; sig->signo!=0; sig++){
		if(sig->signo == signo){
			break;
		}
	}

	action = (char*)"";

	if(proc_type == MASTER_PROC){
		switch(signo){
			case SIGCHLD:
				proc_reap = 1;
				break;
			default:
				break;
		}
	}
	else if(proc_type == WORKER_PROC){

	}
	else{

	}

	if(siginfo && siginfo->si_pid){
		log(LOG,"[SIGNAL] my_signal_handler received, signo: %d, signame: %s, pid: %d, action: %s",
				signo,sig->signame,siginfo->si_pid,action);
	}
	else{
		log(LOG,"[SIGNAL] my_signal_handler received, signo: %d, signame: %s, action: %s",
				signo,sig->signame,action);
	}

	if(signo == SIGCHLD)
	{
		my_process_get_status;
	}

	return;
}

static void my_process_get_status(void){
	pid_t pid;
	int err;
	int status;
	int one = 0;

	for(;;){
		pid = waitpid(-1, &status, WNOHANG);
		if(pid == 0) //子进程未结束
		{
			return;
		}
		else if(pid == -1){ //调用出错
			err = errno;
			if(err == EINTR) //调用被某个信号中断
			{
				continue;
			}
			if(err == ECHILD && one)
			{
				return;
			}
			if(err == ECHILD)
			{
			}
			log(ERROR,"[SIGNAL] my_process_get_status waitpid() failed, errno: %d",err);
			return;
		}
		one = 1;
		if(WTERMSIG(status))
		{
			log(LOG,"[SIGNAL] proc exited on signal, pid: %d, signo: %d",pid,WTERMSIG(status));
		}else{
			log(LOG,"[SIGNAL] proc exited with code, pid: %d, code: %d",pid,WEXITSTATUS(status));
		}
	}
	return;
}
