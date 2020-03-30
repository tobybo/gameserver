#include <strings.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ngx_funs.h"
#include "c_socket.h"
#include "global.h"
#include "macro.h"

using std::string;

CSocket::CSocket(){

}

CSocket::~CSocket(){

}

void CSocket::ReadConf(){
	CConfig *config_instance = CConfig::getInstance();
	m_ListenPortCount = std::stoi((*config_instance)["LISTENPORT_COUNT"]);

}

bool CSocket::setnonblocking(int sockfd){
	int nb = 1;
	if(ioctl(sockfd,FIONBIO,&nb) == -1){
		return false;
	}
	return true;
}

bool CSocket::Initialize(){
	ReadConf();
	return open_listening_sockets();
}

bool CSocket::open_listening_sockets(){
	int isock;
	struct sockaddr_in serv_addr;
	int iport;
	char strinfo[100];

	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	CConfig* config_instance = CConfig::getInstance();
	for(int i = 0; i<m_ListenPortCount; i++){
		isock = socket(AF_INET,SOCK_STREAM,0);
		if(isock == -1){
			log(ERROR,"[SOCKET] open_listening_sockets err,num: %d",i);
			return false;
		}
		int reuseaddr = 1;
		if(setsockopt(isock,SOL_SOCKET,SO_REUSEADDR,(const void*) &reuseaddr, sizeof(reuseaddr)) == -1){
			log(ERROR,"[SOCKET] open_listening_sockets setopt err, num: %d",i);
			close(isock);
			return false;
		}
		if(setnonblocking(isock) == false){
			log(ERROR,"[SOCKET] open_listening_sockets setlok err, num: %d",i);
			close(isock);
			return false;
		}
		strinfo[0] = 0;
		sprintf(strinfo,"LISTENPORT%d",i);
		iport = std::stoi((*config_instance)[string(strinfo)]);
		serv_addr.sin_port = htons(iport);

		if(bind(isock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
			log(ERROR,"[SOCKET] open_listening_sockets bind err, num: %d",i);
			close(isock);
			return false;
		}

		if(listen(isock, LISTEN_BACKLOG)==-1){
			log(ERROR,"[SOCKET] open_listening_sockets listen err, num: %d",i);
			close(isock);
			return false;
		}

		lp_listening_t p_listensocketitem = new listening_t;
		bzero(p_listensocketitem,sizeof(listening_t));
		p_listensocketitem -> port = iport;
		p_listensocketitem -> fd = isock;
		p_listensocketitem -> connection = nullptr;
		log(LOG,"[SOCKET] open_listening_sockets succ, num: %d, port: %d",i,iport);
		m_ListenSocketList.push_back(p_listensocketitem);
	}
	if(m_ListenSocketList.size() <= 0)
	{
		return false;
	}
	return true;
}

bool CSocket::Initialize_subporc(){
	return true;
}

void CSocket::Shutdown_subproc(){

	return;
}

int CSocket::epoll_process_events(int timer){

	return 1;
}
