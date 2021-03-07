#ifndef RW_H
#define RW_H
#include <iostream>  
#include <fstream>  
#include <sstream>  
#include <cstring>  
#include <vector> 
#include<iterator> 
#include<stdlib.h>  
#define _IP_MARK "."
using namespace std;  


//IP转换成int
unsigned int IPtoINT(const string & strIP)
{
    unsigned int nRet = 0;
    char chBuf[16] = "";
    memcpy(chBuf, strIP.c_str(), 15); //memcpy_s和strtok_s是windows下的安全函数  由src所指向内存区域复制count个字节到dest指向的内存区域。
    char* szBufTemp = NULL;
    char* szBuf = strtok_r(chBuf,_IP_MARK,&szBufTemp); 
    int i = 0;//计数
    while(NULL != szBuf)
    {
        nRet += atoi(szBuf)<<((3-i)*8);
        szBuf = strtok_r(NULL,_IP_MARK,&szBufTemp);//
        i++;
    }

    return nRet;
}

//read_csv
vector<vector<unsigned int> > read_data(const char *filename){
    vector<vector<unsigned int> > strArray;  //vector是一个能够存放任意类型的动态数组
    string lineStr;
    ifstream infile(filename, ios::in);  
    getline(infile, lineStr);//除去第一行的字段信息
    while (getline(infile, lineStr))  
    {  
        // 存成二维表结构  
        stringstream ss(lineStr);  //stringstream可用于字符串分割
        string str;  
        vector<unsigned int> lineArray;  
        
        for(int i=0; i<6;i++){
            getline(ss, str, ',');
            if(i<2){
                lineArray.push_back(IPtoINT(str));
            }else{
                lineArray.push_back(atoi(str.c_str())); 
            }

        }
        
        strArray.push_back(lineArray);  
                
    }  

    return strArray;
}

//save_csv
void save_data(vector< int> estArray,const char *savepath){
    ofstream output_file(savepath);
    ostream_iterator< int> output_iterator(output_file, "\n");
    std::copy(estArray.begin(), estArray.end(), output_iterator);
    output_file.close();
}


#endif