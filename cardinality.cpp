#ifndef RADE_H
#define RADE_H
#include <iostream>
#include <iomanip>
#include <fstream>
#include "datatypes.hpp"
#include "adaptor.cpp"
#include "adaptor.hpp"
#include "pipesketch.cpp"
#include "pipesketch.hpp"
#include <bitset>
#include <cmath>
#include "CM/cmsketch.cpp"
// #include "CU/cusketch.cpp"
#include "LinearCounting.cpp"
int main(int argc, char* argv[]){

    //Dataset filename
    const char* filenames = "iptraces.txt"; 
    unsigned long long buf_size = 50000000;//buffer缓存区域 8bytes buf_size可容纳37GB的数据 5000000000
    //设置参数->根据内存大小进行计算
    
    int memory = 120; //KB 100 140 180 220 260 300 


    int lgwidth = (memory*1024*0.86)/(4*2);//0.86 0.09  0.05
    int rgwidth = (memory*1024*0.09)/(4*3); //0.625 0.34 0.035 1024*
    int hvwidth = (memory*1024*0.05)/21; //
    std::cout<<"lgwidth:"<<lgwidth<<std::endl;
    // cm
    int cmwidth = 13209;//(memory*1024)/(4*4);//90400 bytes=88kb
    // int cmdepth = 4; 
    std::cout<<"cm_memory:"<<(cmwidth*14*1.0/1024)<<std::endl;
    //cu
    int cuwidth = (memory*1024)/(4*4);
    // int cudepth = 4;

    std::ifstream tracefiles(filenames);//定义类tracefiles,用来读取文件
    if (!tracefiles.is_open()){
        std::cout << "Error opening file" << std::endl;
        return -1;
    }
    std::cout << "opening file successful" << std::endl;

    for (std::string file; getline(tracefiles, file);){

        //load traces
        std::cout << "load traces" << std::endl;
        std::cout << "[Dataset]: " << file << std::endl;
        Adaptor* adaptor =  new Adaptor(file, buf_size); //file为traces/example.pcap,adaptor读取并解析pcap文件
        std::cout << "[Message] Finish read data." << std::endl;
        
        adaptor->Reset();
        mymap ground;
        val_tp totalSize = 0;
        uint32_t pktCount = 0;//数据包个数
        tuple_t t;
        
        while(adaptor->GetNext(&t) == 1){
            totalSize += t.size;
            pktCount ++;
            key_tp key;
            memcpy(key.key, &(t.key), 13);
            if (ground.find(key) != ground.end()){
                ground[key] += t.size;
            }else{
                ground[key] = t.size;
            }
        }

        PipeSketch* pipe = new PipeSketch(lgwidth,rgwidth,hvwidth);
        CMSketch* cm = new CMSketch(cmwidth);
        LinearCounting* lc = new LinearCounting(cmwidth*4);
        // CUSketch* cu = new CUSketch(cuwidth);
        adaptor->Reset();
        memset(&t, 0, sizeof(tuple_t));
        while(adaptor->GetNext(&t) == 1){
            pipe->insert((unsigned char*)&(t.key),t.size);
            cm->insert((unsigned char*)&(t.key),t.size); 
            lc->insert((unsigned char*)&(t.key),t.size);
            // cu->insert((unsigned char*)&(t.key),t.size);  
        }
       
        uint32_t card = pipe->getCardinality();
        uint32_t cm_card = cm->getCardinality();
        uint32_t lc_card = lc->getCardinality();
        std::cout<<"card:"<<card<<std::endl;
        std::cout<<"cm_card:"<<cm_card<<std::endl;
        std::cout<<"lc_card:"<<lc_card<<std::endl;
        std::cout<<"true card:"<<ground.size()<<std::endl;

        delete pipe; 
        // delete cm;
        // delete cu;
        delete adaptor;

    }
    return 0;
}
#endif
