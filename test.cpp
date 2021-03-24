#include <iostream>
using namespace std;
int main(){

    int memory = 100; //KB
    int lgwidth = (memory*1024*0.8)/(2*4);
    int rgwidth = (memory*1024*0.18)/(4*3);
    int hvwidth = (memory*1024*0.02)/4;
    int cmwidth = (memory*1024)/(4*4);

    cout<<"lgwidth"<<lgwidth<<endl;
    cout<<"rgwidth"<<rgwidth<<endl;
    cout<<"hvwidth"<<hvwidth<<endl;
    cout<<"cmwidth"<<cmwidth<<endl;
    return 0;
}



// std::cout<<"query result:"<<pipe_est<<"true size:"<<it->second<<std::endl;
// std::cout<<"err:"<<pipe_err<<std::endl;
 /*
std::cout<<"1:"<<std::bitset<8>(key.key[3])<<" "<<std::bitset<8>(key.key[2])<<" "<<std::bitset<8>(key.key[1])<<" "<<std::bitset<8>(key.key[0])<<std::endl;
std::cout<<"2:"<<std::bitset<8>(key.key[7])<<" "<<std::bitset<8>(key.key[6])<<" "<<std::bitset<8>(key.key[5])<<" "<<std::bitset<8>(key.key[4])<<std::endl;
std::cout<<"3:"<<std::bitset<8>(key.key[9])<<" "<<std::bitset<8>(key.key[8])<<std::endl;
std::cout<<"4:"<<std::bitset<8>(key.key[11])<<" "<<std::bitset<8>(key.key[10])<<std::endl;
std::cout<<"5:"<<std::bitset<8>(key.key[12])<<std::endl;
*/
 /*
   //heavy hiter
        
        double thresh = 0.0008;
        val_tp hh_threshold = thresh*totalSize;
        
        std::vector<std::pair<key_tp, val_tp> > hh_results;
        pipe->getHeavyHitter(hh_threshold, hh_results);
        //统计AAE ARE precision recall
        double hh_precision = 0, hh_recall=0, hh_error=0, hh_throughput=0;//detectime=0;
        int hh_tp = 0, hh_cnt = 0;;
        for (auto it = ground.begin(); it != ground.end(); it++) {
        //ground:原始值 results：估计值
            if (it->second > hh_threshold) {
                hh_cnt++;
                for (auto res = hh_results.begin(); res != hh_results.end(); res++) {
                    if (memcmp(it->first.key, res->first.key, sizeof(res->first.key)) == 0) {

                        std::cout<<"true:"<<it->second<<" "<<"est:"<<res->second<<" "<<"diff:"<<int(res->second - it->second)<<std::endl;
                        hh_error += std::abs(int(res->second - it->second))*1.0;//it->second)
                        std::cout<<"hh_error:"<<hh_error<<std::endl;
                        hh_tp++;
                    }
                }
            }
        }
        std::cout<<"hh_error:"<<hh_error<<"hh_tp:"<<hh_tp<<std::endl;
        // std::cout<<"total bytes:"<<totalSize<<std::endl;
        // std::cout<<"total pkts:"<<pktCount<<std::endl; 
        std::cout<<"thresh:"<<hh_threshold<<std::endl; 
        std::cout<<"results size:"<<hh_results.size()<<std::endl;
        hh_precision = hh_tp*1.0/hh_results.size(); //pre=tp/tp+fp recall=tp/tp+fn
        hh_recall = hh_tp*1.0/hh_cnt;
        hh_error = hh_error/hh_tp;//平均误差

       
        std::cout<<"hh_precision:"<<hh_precision<<std::endl;
        std::cout<<"hh_recall:"<<hh_recall<<std::endl;
        
       
        //cardinality
        uint32_t card = pipe->getCardinality();
        std::cout<<"card:"<<card<<std::endl;
        std::cout<<"true card:"<<ground.size()<<std::endl;
        
        //flow size distribution
        vector<double> dist;
        std::cout<<"get distribution."<<std::endl;
        pipe->get_distribution(dist);
        std::cout<<"finish get distribution."<<std::endl;
        for(int i = 0, j = 0; i < (int)dist.size(); ++i){
            std::cout<<"flow size distribution:"<<i<<" "<<dist[i]<<std::endl;
        }
        */