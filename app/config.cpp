#include<stdlib.h>
#include<cstring>
#include<iostream>
#include<fstream>
#include "ngx_funs.h"
#include "config.h"

using std::cout;
using std::endl;
//using std::string;

CConfig* CConfig::m_instance = NULL;

CConfig::CConfig(){

}

CConfig::~CConfig(){
    auto pos = m_info.begin();
    for(;pos<m_info.end();pos++){
        delete (*pos);
    }
    m_info.clear();
}

bool CConfig::loadConfig(const string& filename){
    std::fstream fs;
    /*c++98 void open (const   char* filename,  ios_base::openmode mode = ios_base::in);*/
    /*c++11特性 void open (const string& filename,  ios_base::openmode mode = ios_base::in);*/
    fs.open(filename,std::fstream::in);
	if(!fs)
		cout<<"打开配置文件失败: "<<filename<<endl;
    char line_buff[501];
    int len_s;
	char* temp1;
    while(fs.getline(line_buff,500)&&fs){
        char* temp = trimStrLeft(line_buff,strlen(line_buff),' ');
		trimStrRight(temp,strlen(temp),' ');
		temp1 = strchr(temp,'=');
		if(nullptr==temp1)
		{
			continue;
		}
		*temp1 = '\0';
		temp1++;
		SConfInfoPtr info_ptr = new SConfInfo;
		trimStrRight(temp,strlen(temp),' ');
		temp1 = trimStrLeft(temp1,strlen(temp1),' ');
		info_ptr->name = temp;
		info_ptr->content = temp1;
	 	m_info.push_back(info_ptr);
    }
    fs.close();
	showInfo();
    return true;
}

const string CConfig::getStrContent(const string& name) const{
    auto pos = m_info.begin();
    for(;pos<m_info.end();pos++){
        if((*pos)->name == name)
            return (*pos)->content;
    }
    return "";
}

const int CConfig::getIntContent(const string& name) const{
    auto pos = m_info.begin();
    for(;pos<m_info.end();pos++){
        if((*pos)->name == name)
            return std::stoi((*pos)->content);
    }
    return -1;
}

void CConfig::showInfo() const{
	auto pos = m_info.begin();
	for(;pos<m_info.end();pos++)
	{
		cout<<(*pos)->name<<":"<<(*pos)->content<<endl;
	}
}

const string CConfig::operator[](const string& name) const{
	string content = getStrContent(name);
	return content;
}
