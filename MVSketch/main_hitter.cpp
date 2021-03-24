#include "mvsketch.hpp"
#include "adaptor.hpp"
#include <unordered_map>
#include <utility>
#include <iomanip>
#include "datatypes.hpp"
#include "util.h"


int main(int argc, char* argv[]) {



    //Confiture parameter

    //Dataset filename
    const char* filenames = "iptraces.txt";
    unsigned long long buf_size = 5000000000;//buffer缓存区域
    //Heavy hitter threshold
    double thresh = 0.0008;

    //mv sketch parameters
    int mv_width = 1366; //number of buckets in each row
    int mv_depth = 4; //number of rows

    //evaluation
    std::vector<std::pair<key_tp, val_tp> > results;
    int numfile = 0;
    double precision = 0, recall=0, error=0, throughput=0, detectime=0;
    double avpre=0, avrec=0, averr=0, avthr=0, avdet=0;

    std::ifstream tracefiles(filenames);//定义类tracefiles,用来读取文件
    if (!tracefiles.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return -1;
    }

    for (std::string file; getline(tracefiles, file);) {

        //load traces
        Adaptor* adaptor =  new Adaptor(file, buf_size);
        std::cout << "[Dataset]: " << file << std::endl;
        std::cout << "[Message] Finish read data." << std::endl;


        //Get ground
        adaptor->Reset();
        mymap ground;
        val_tp sum = 0;
        tuple_t t;
        while(adaptor->GetNext(&t) == 1) {
            sum += t.size;
            key_tp key;
            memcpy(key.key, &(t.key), LGN);//常量LGB大小为8
            if (ground.find(key) != ground.end()) {
                ground[key] += t.size;
            } else {
                ground[key] = t.size;//给mymap中的value赋值
            }
        }
        std::cout << "[Message] Finish Insert hash table" << std::endl;
        val_tp threshold = thresh*sum;

        //Create sketch
        MVSketch* mv = new MVSketch(mv_depth, mv_width, 8*LGN);

        //Update sketch
        uint64_t t1=0, t2=0;
        adaptor->Reset();
        memset(&t, 0, sizeof(tuple_t));//初始化工作tuple_t(key,size)
        t1 = now_us();//获取时间
        while(adaptor->GetNext(&t) == 1) {
            mv->Update((unsigned char*)&(t.key), (val_tp)t.size);
        }
        t2 = now_us();
        throughput = adaptor->GetDataSize()/(double)(t2-t1)*1000000000;//计算吞吐量 数据包个数/t2-t1 t的单位为ns 1ns=10^-9 s
        std::cout << "time = " << (double)(t2-t1)*1000000000 << std::endl;
        //Query sketch
        results.clear();
        t1 = now_us();
        mv->Query(threshold, results);
        t2 = now_us();
        detectime = (double)(t2-t1)/1000000000;

        //Evaluate accuracy cnt字节数超过阈值的flow的数目
        int tp = 0, cnt = 0;;
        error = 0;
        for (auto it = ground.begin(); it != ground.end(); it++) {//ground:原始值 results：估计值
            if (it->second > threshold) {
                cnt++;
                for (auto res = results.begin(); res != results.end(); res++) {
                    if (memcmp(it->first.key, res->first.key, sizeof(res->first.key)) == 0) {
                        error += fabs(res->second - it->second)*1.0/it->second;
                        tp++;
                    }
                }
            }
        }
        precision = tp*1.0/results.size(); //pre=tp/tp+fp recall=tp/tp+fn
        recall = tp*1.0/cnt;
        error = error/tp;//平均误差

        avpre += precision; avrec += recall; averr += error; avthr += throughput; avdet += detectime;
        delete mv;
        delete adaptor;

        numfile++;
        //Output to standard output
        std::cout << std::setfill(' ');
        std::cout << std::setw(20) << std::left << "Sketch" << std::setw(20) << std::left << "Precision"
            << std::setw(20) << std::left << "Recall" << std::setw(20)
            << std::left << "Relative Error" << std::setw(20) << std::left << "Throughput" << std::setw(20)
            << std::left << "Detection Time" << std::endl;
            std::cout << std::setw(20) <<  "MVSketch"
            << std::setw(20) << std::left << precision << std::setw(20) << std::left << recall << std::setw(20)
                << std::left << error << std::setw(20) << std::left << throughput << std::setw(20)
                << std::left << detectime << std::endl;


    }

            //Output to standard output
        std::cout << "---------------------------------  summary ---------------------------------" << std::endl;
        std::cout << std::setfill(' ');
        std::cout << std::setw(20) << std::left << "Sketch" << std::setw(20) << std::left << "Precision"
            << std::setw(20) << std::left << "Recall" << std::setw(20)
            << std::left << "Relative Error" << std::setw(20) << std::left << "Throughput" << std::setw(20)
            << std::left << "Detection Time" << std::endl;
            std::cout << std::setw(20) <<  "MVSketch" << std::setw(20) << std::left << avpre/numfile << std::setw(20) << std::left << avrec/numfile << std::setw(20)
                << std::left << averr/numfile << std::setw(20) << std::left << avthr/numfile << std::setw(20)
                << std::left << avdet/numfile << std::endl;

}
