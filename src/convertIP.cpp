
#include <cstring>
#include <iostream>
#include<string.h>
using namespace std;  

#define _IP_MARK "."

//IP转换成int
unsigned int IPtoINT(const string & strIP)
{
    unsigned int nRet = 0;

    char chBuf[16] = "";
    memcpy(chBuf, strIP.c_str(), 15); //memcpy_s和strtok_s是windows下的安全函数  由src所指向内存区域复制count个字节到dest指向的内存区域。
    char* szBufTemp = NULL;
    char* szBuf = strtok_r(chBuf,_IP_MARK,&szBufTemp); 
     // cout << szBufTemp <<end;
    int i = 0;//计数
    while(NULL != szBuf)//取一个
    {
        nRet += atoi(szBuf)<<((3-i)*8);
        szBuf = strtok_r(NULL,_IP_MARK,&szBufTemp);//
        i++;
    }

    return nRet;
}

int main()  
{  
    string ipstring = "0";
    unsigned int srcIP = IPtoINT(ipstring);
    cout << srcIP<<endl;
}