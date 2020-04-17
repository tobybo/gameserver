#include<string.h>
#include<iostream>

static const char* invalid_string = R"+*("(*,;/\%?#<> .,|'")")+*";

void testDefFun(){
    std::cout<<"this is testDefFun!"<<std::endl;
}

/*
 *len: strlen()
 * */
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

/*
 *len: strlen()
 * */
void trimStrRight(char* s,int len,const char c){
    for(int i = len - 1;i>=0;i--){
        if(s[i] == c)
            s[i] = '\0';
        else
            break;
    }
}

/*
 *len: strlen()
 * */
char* trimStr(char* s, const char c){
	char* temp = trimStrLeft(s, strlen(s), c);
	trimStrRight(temp, strlen(temp), c);
	return temp;
}

char* trimStr(char* s){
	char c = ' ';
	char* temp = trimStrLeft(s, strlen(s), c);
	trimStrRight(temp, strlen(temp), c);
	return temp;
}

/*
 *len: strlen()
 * */
bool checkStr(char* s, int len)
{
	for(int i = 0; i < len - 1; i++)
	{
		if(strchr(invalid_string,s[i]))
		{
			return false;
		}
	}
	return true;
}

