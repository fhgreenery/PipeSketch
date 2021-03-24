#ifndef HITTER_H
#define HITTER_H
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
#include <unordered_map>
#include <vector>
#include "CM/cm_heap.cpp"
#include "MVSketch2/mvsketch.cpp"
// #include "CU/cusketch.cpp"

int main(int argc, char* argv[]){

	//Dataset filename
    const char* filenames = "iptraces.txt"; 
    unsigned long long buf_size = 50000000;//buffer缓存区域 8bytes buf_size可容纳37GB的数据 5000000000
    
    int memory = 100; //KB 100 140 180 220 260 300 

    int lgwidth = (memory*1024*0.86)/(4*2);//665 335 1024*
    int rgwidth = (memory*1024*0.09)/(4*3); //0.625 0.34 0.035 1024*
    int hvwidth = (memory*1024*0.05)/21; // 4876
    std::cout<<"hvwidth:"<<hvwidth<<std::endl;
    int lgdepth = 4;
    int rgdepth = 3;
    int hvdepth = 1; 

    // cm
    int cmwidth = (memory*1024)/(4*4);//90400 bytes=88kb
    int cmdepth = 4; 

    //cu
    int cuwidth = (memory*1024)/(4*4);
    int cudepth = 4;

    //mv //和hvwidth一样
    int mvwidth = 300; 
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
        //heavy hitter
        PipeSketch* pipe = new PipeSketch(lgwidth,rgwidth,hvwidth);
        CMH* cm = new CMH(cmwidth);
        MVSketch* mv = new MVSketch(4, mvwidth, 13*8);
        adaptor->Reset();
        memset(&t, 0, sizeof(tuple_t));
        while(adaptor->GetNext(&t) == 1){
            pipe->insert((unsigned char*)&(t.key),t.size);
            cm->insert((unsigned char*)&(t.key),t.size);
            mv->Update((unsigned char*)&(t.key), t.size);
        }
        double thresh = 0.0008;
        val_tp hh_threshold = thresh*totalSize;
        
        std::vector<std::pair<key_tp, val_tp> > hh_results;
        pipe->getHeavyHitter(hh_threshold, hh_results);

        std::vector< std::pair<key_tp, val_tp> > heavy_hitters;
        cm->get_heavy_hitters(hh_threshold,heavy_hitters);
        std::vector<std::pair<key_tp, val_tp> > mv_hitters;
        mv->Query(hh_threshold, mv_hitters);
        //统计AAE ARE precision recall
        double hh_precision = 0, hh_recall=0, hh_error=0, hh_throughput=0;//detectime=0;
        double cm_precision = 0, cm_recall=0, cm_error=0, cm_throughput=0;
        double mv_precision = 0, mv_recall=0, mv_error=0, mv_throughput=0;
        int hh_tp = 0, hh_cnt = 0;
        int cm_tp = 0;
        int mv_tp = 0;
        for (auto it = ground.begin(); it != ground.end(); it++) {
            if (it->second > hh_threshold) {
                hh_cnt++;
                for (auto res = hh_results.begin(); res != hh_results.end(); res++) {
                    if (memcmp(it->first.key, res->first.key, sizeof(res->first.key)) == 0) {

                        // std::cout<<"true:"<<it->second<<" "<<"est:"<<res->second<<" "<<"diff:"<<int(res->second - it->second)<<std::endl;
                        hh_error += std::abs(int(res->second - it->second))*1.0;//it->second)
                        // std::cout<<"hh_error:"<<hh_error<<std::endl;
                        hh_tp++;
                    }
                }

                for (auto res = heavy_hitters.begin(); res != heavy_hitters.end(); res++) {
                    if (memcmp(it->first.key, res->first.key, sizeof(res->first.key)) == 0) {

                        // std::cout<<"true:"<<it->second<<" "<<"est:"<<res->second<<" "<<"diff:"<<int(res->second - it->second)<<std::endl;
                        cm_error += std::abs(int(res->second - it->second))*1.0;//it->second)
                        // std::cout<<"hh_error:"<<hh_error<<std::endl;
                        cm_tp++;
                    }
                }

                for (auto res = mv_hitters.begin(); res != mv_hitters.end(); res++) {
                    if (memcmp(it->first.key, res->first.key, sizeof(res->first.key)) == 0) {

                        // std::cout<<"true:"<<it->second<<" "<<"est:"<<res->second<<" "<<"diff:"<<int(res->second - it->second)<<std::endl;
                        mv_error += std::abs(int(res->second - it->second))*1.0;//it->second)
                        // std::cout<<"hh_error:"<<hh_error<<std::endl;
                        mv_tp++;
                    }
                }
            }
        }
        // std::cout<<"hh_error:"<<hh_error<<"hh_tp:"<<hh_tp<<std::endl;
        // std::cout<<"thresh:"<<hh_threshold<<std::endl; 
        // std::cout<<"results size:"<<hh_results.size()<<std::endl;
        hh_precision = hh_tp*1.0/hh_results.size(); //pre=tp/tp+fp recall=tp/tp+fn
        hh_recall = hh_tp*1.0/hh_cnt;
        hh_error = hh_error/hh_tp;//平均误差

        cm_precision = cm_tp*1.0/heavy_hitters.size(); //pre=tp/tp+fp recall=tp/tp+fn
        cm_recall = cm_tp*1.0/hh_cnt;
        cm_error = cm_error/cm_tp;
       
        cm_precision = mv_tp*1.0/mv_hitters.size(); //pre=tp/tp+fp recall=tp/tp+fn
        cm_recall = mv_tp*1.0/hh_cnt;
        cm_error = mv_error/mv_tp;

        std::cout<<"hh_precision:"<<hh_precision<<std::endl;
        std::cout<<"hh_recall:"<<hh_recall<<std::endl;
        std::cout<<"cm_precision:"<<cm_precision<<std::endl;
        std::cout<<"cm_recall:"<<cm_recall<<std::endl;

        std::cout<<"mv_precision:"<<mv_precision<<std::endl;
        std::cout<<"mv_recall:"<<mv_recall<<std::endl;

        delete pipe; 
        // delete cm;
        // delete cu;
        delete adaptor;

    }
    return 0;
}
#endif

