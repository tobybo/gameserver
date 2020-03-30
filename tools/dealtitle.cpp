//处理进程标题
#include<stdlib.h>
#include<unistd.h>
#include<cstring>
#include<iostream>
#include "global.h"
#include "macro.h"
#include "ngx_funs.h"

//先把环境变量挪到新分配的内存空间
void title_move_environ(){
	//1 计算需要的内存大小
	int i = 0;
	for(;environ[i];i++){
		gp_environlen += strlen(environ[i]) + 1;
	}
	gp_envmem = new char[gp_environlen];
	char *temp = gp_envmem;
	size_t len_s;
	for(i = 0;environ[i];i++){
		len_s = strlen(environ[i]) + 1;
		memcpy(temp,environ[i],len_s);
		environ[i] = temp;
		temp += len_s;
	}
}

void title_set(const char *title)
{
	using std::cout;
	using std::endl;
	log(LOG,"[TITLE] title_set begin, title: %s",title);
	//cout<<"begin set title: "<<title<<endl;
	size_t arg_len = 0;
	size_t title_len = strlen(title) + 1;
	if(title_len > gp_environlen)
	{
		std::cout<<"error: title_set failed,too long"<<std::endl;
		return;
	}

	for(int i=0;g_os_arg[i];i++){
		arg_len += strlen(g_os_arg[i]) + 1;
	}

	char *temp = g_os_arg[0];
	for(int i=title_len+arg_len;i>title_len - 1;i--)
	{
		temp[i] = temp[i - title_len];
		if(temp[i]=='\0')
			temp[i] = ' ';
	}
	strcpy(temp,title);
	temp[title_len + arg_len] = '\0';
	temp = temp+title_len+arg_len;
	memset(temp,0,gp_environlen-title_len);
	//cout<<"end set title: "<<title<<endl;
	log(LOG,"[TITLE] title_set end, title: %s",title);
}
