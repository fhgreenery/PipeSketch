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
#include "CU/cusketch.cpp"
#include "util.h"
int main(int argc, char* argv[]){

	//Dataset filename
    const char* filenames = "iptraces.txt"; 
    unsigned long long buf_size = 50000000;//buffer缓存区域 8bytes buf_size可容纳37GB的数据 5000000000
    //设置参数->根据内存大小进行计算
    
    int memory = 140; //KB 100 140 180 220 260 300 

    int lgwidth = (memory*1024*0.94)/(4*2);//0.86 0.09  0.05
    int rgwidth = (memory*1024*0.06)/(4*3); //0.625 0.34 0.035 1024*
    int hvwidth = (memory*1024*0.05)/21; //

    // cm
    int cmwidth = (memory*1024)/(4*4);//90400 bytes=88kb
    // int cmdepth = 4; 

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
        
        //统计0-65535的数据流的数量
        /*
        int inCount = 0;
        for (auto it = ground.begin(); it != ground.end(); it++) { //对每个流进行统计
            if(it->second < 65535){ inCount++;} 
        }
        float inpercent = inCount*1.0/ground.size();
        std::cout << "percent:" <<inpercent<<" "<<"inCount:"<<inCount<<" "<<ground.size()<<std::endl;
        
        */
        uint64_t t1=0, t2=0,t3=0,t4=0;

        PipeSketch* pipe = new PipeSketch(lgwidth,rgwidth,hvwidth);
        CMSketch* cm = new CMSketch(cmwidth);
        CUSketch* cu = new CUSketch(cuwidth);
        adaptor->Reset();
        memset(&t, 0, sizeof(tuple_t));
        while(adaptor->GetNext(&t) == 1){
            t1 = now_us();//获取时间
            pipe->insert((unsigned char*)&(t.key),t.size);
            t2 = now_us();
            cm->insert((unsigned char*)&(t.key),t.size); 
            t3 = now_us();
            cu->insert((unsigned char*)&(t.key),t.size);  
            t4 = now_us();
        }
        double pipe_th = pktCount/(double)(t2-t1)*1000000000;
        double cm_th = pktCount/(double)(t3-t2)*1000000000;
        double cu_th = pktCount/(double)(t4-t3)*1000000000;

        uint32_t pipe_err=0,cm_err=0,cu_err=0;
        uint32_t pipe_errs=0, cm_errs=0,cu_errs=0;
        float pipe_ares=0, cm_ares=0,cu_ares=0;
        uint32_t pipe_est=0,cm_est=0,cu_est=0;
        // int temcnt = 0;
        for (auto it = ground.begin(); it != ground.end(); it++) { //对每个流进行统计
            pipe_est = pipe->query((unsigned char*)&(it->first.key));
            cm_est = cm->query((unsigned char*)&(it->first.key));
            cu_est = cu->query((unsigned char*)&(it->first.key));
            pipe_err = std::abs(int(pipe_est - it->second));
            cm_err = std::abs(int(cm_est - it->second));
            cu_err = std::abs(int(cu_est - it->second));
            pipe_ares += pipe_err*1.0/it->second;
            cm_ares += cm_err*1.0/it->second;
            cu_ares += cu_err*1.0/it->second;
            pipe_errs += pipe_err;
            // if(temcnt == 2060){
            //     std::cout<<temcnt<<" "<<"pipe_est:"<<pipe_est<<std::endl;
            // }
            
            cm_errs += cm_err;
            cu_errs += cu_err;
            // temcnt ++;
        }
        //应当四舍五入，保留两位小数
        int flowCount = ground.size();
        float pipe_AAE = pipe_errs*1.0/flowCount;
        float cm_AAE = cm_errs*1.0/flowCount;
        float cu_AAE = cu_errs*1.0/flowCount;
        float pipe_ARE = pipe_ares*1.0/flowCount;
        float cm_ARE = cm_ares*1.0/flowCount;
        float cu_ARE = cu_ares*1.0/flowCount;
        std::cout<<"pipe_AAE:"<<pipe_AAE<<std::endl;
        std::cout<<"cm_AAE:"<<cm_AAE<<std::endl;
        std::cout<<"cu_AAE:"<<cu_AAE<<std::endl;
        std::cout<<"pipe_ARE:"<<pipe_ARE<<std::endl;
        std::cout<<"cm_ARE:"<<cm_ARE<<std::endl;
        std::cout<<"cu_ARE:"<<cu_ARE<<std::endl;
        //thoughout
        std::cout<<"pipe_th:"<<pipe_th<<std::endl;
        std::cout<<"cm_th:"<<cu_th<<std::endl;
        std::cout<<"cu_th:"<<cu_th<<std::endl;

        delete pipe; 
        delete cm;
        delete cu;
        delete adaptor;

    }
    return 0;
}
#endif
