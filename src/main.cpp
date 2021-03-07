#include <iostream>  
#include <string>  
#include <cstring>  
#include <vector>  
#include<stdlib.h>  
#include "datatypes.hpp"
// #include "pipesketch.hpp"
#include "pipesketch.cpp"
#include "rwdata.hpp"
// #include <unordered_map>
// #include<map>
// #include "BOBHash32.h"
#include <fstream>  
#include <sstream>  
using namespace std;  

 //设置参数
int lgwidth = 4000;
int lgdepth = 4;

int rgwidth = 1000;
int rgdepth = 3;

int hvwidth = 300;
int hvdepth = 1; 


int main()  
{  
    pkt t= {0}; 
    vector<vector<unsigned int> > dataStream1, dataStream2;
    vector<int>  estArray;

    // PipeSketch* pipe = new PipeSketch(lgwidth,lgdepth,rgwidth,rgdepth,hvwidth,hvdepth);
    //insert
    dataStream1 = read_data("../data/trace_5w/test.csv");

    // pipe->~PipeSketch();
    return 0;  
}


 /*
  -std=gnu++11
map<uint,int> true_map;

uint bobhash(char* key){
    BOBHash32* bobhash = new BOBHash32(500);
    return bobhash->run(key,13);
}
    uint hashvalue;
    vector<unsigned int> trueArray;
    hashvalue = bobhash((char*)&(t.key));
    true_map[hashvalue] += t.pkt_size;
     for (auto it = true_map.begin(); it != true_map.end(); it++) {
        trueArray.push_back(it->second);
     }
     save_data(trueArray,"../data/trace_5w/trace_5w_query_true.csv"); //map.size() 数据流数量
     */


 /*
    for (vector< vector<unsigned int> >::iterator it = dataStream1.begin(); it != dataStream1.end(); ++it){//二维数组
        // for (int i = 0; i < (*it).size(); ++i){}
        t.key.src_ip = (*it)[0]; 
        t.key.dst_ip = (*it)[1]; 
        t.key.src_port = (*it)[2];
        t.key.dst_port = (*it)[3];
        t.key.protocol = (*it)[4];
        t.pkt_size = (*it)[5];
        pipe->insert((char*)&(t.key),t.pkt_size);
    }
    */
    //query 
    /*
    pkt m;
    
    dataStream2 = read_data("../data/trace_5w/trace_5w_true.csv");
    for (vector< vector<unsigned int> >::iterator it = dataStream2.begin(); it != dataStream2.end(); ++it){
        m.key.src_ip = (*it)[0]; 
        m.key.dst_ip = (*it)[1];
        m.key.src_port = (*it)[2];
        m.key.dst_port = (*it)[3];
        m.key.protocol = (*it)[4];
       
        int32_t result = pipe->query((char*)&(m.key));
        estArray.push_back(result);
      
    }  

    cout <<"query success."<<endl;
    
    save_data(estArray,"../data/trace_5w/trace_5w_query5.csv");
    estArray.clear();
    */