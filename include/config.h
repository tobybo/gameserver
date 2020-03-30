#ifndef _CONFIG_H_
#define _CONFIG_H_

#include<vector>
#include<string>

#include "ngx_funs.h"

using std::string;

class CConfig
{
    private:
        CConfig(); //隐藏构造函数
    public:
        ~CConfig();
    private:
        std::vector<SConfInfoPtr> m_info;
        static CConfig* m_instance;
    public:
        static CConfig* getInstance(){
            if(!m_instance)
            {
                m_instance = new CConfig;
            }
            return m_instance;
        }
    private:
        class CConfigDelete
        {
            ~CConfigDelete(){
                if(CConfig::m_instance){
                    delete CConfig::m_instance;
                    CConfig::m_instance = NULL;
                }
            }
        };
    public:
        bool loadConfig(const string& filename);
        //这里没有接收返回值的引用 是因为有可能没有查找到对应配置 没有有效的数据的引用返回
        const string getStrContent(const string& name) const;
        const int getIntContent(const string& name) const;
        void showInfo() const;
        const string operator[](const string& name) const;
};

#endif
