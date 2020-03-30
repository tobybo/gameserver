#include<iostream>

void testDefFun(){
    std::cout<<"this is testDefFun!"<<std::endl;
}


char* trimStrLeft(char* s,int len,const char c){
    char* temp = s;
    for(int i = 0;i<len;i++){
        if(s[i] == c)
            temp++;
        else
            break;
    }
    return temp;
}

void trimStrRight(char* s,int len,const char c){
    for(int i = len - 1;i>=0;i--){
        if(s[i] == c)
            s[i] = '\0';
        else
            break;
    }
}
