#ifndef CMSKETCH_H
#define CMSKETCH_H
#include "../hash.h"


class CMSketch{

	typedef struct BUCKET_type {
        int32_t bsize;
    } Bucket;

   struct CM_type {
        int depth = 4;
        int width;
        uint64_t seed[4] = {541,1211,1862,1094};
        Bucket* cm_table[4];//定义一个指针，用来存储二维数组
    };

    
	private:
		CM_type cm;


	public:
		CMSketch(int width){
			cm.width = width;
			for (int i = 0; i < cm.depth; i++){
				cm.cm_table[i] = new Bucket[width];
				memset(cm.cm_table[i], 0, width*sizeof(Bucket));
			}
		}

		~CMSketch(){
			for (int i = 0; i < cm.depth; i++)
        		delete cm.cm_table[i];
			
		}
		void insert(unsigned char* key, int32_t pkt_size){

			int index = 0;
			for(int i=0; i<cm.depth; i++){ 
		        index = MurmurHash64A(key, 13, cm.seed[i])% cm.width;	       
		        cm.cm_table[i][index].bsize += pkt_size;
			}
			
		}
		uint32_t query(unsigned char* key){
			int index = 0;
			uint32_t min_est = 2147483647; //2^31-1
			for(int i=0; i<cm.depth; i++){
		        index = MurmurHash64A(key, 13, cm.seed[i]) % cm.width;
		        if(cm.cm_table[i][index].bsize < min_est){
		        	min_est = cm.cm_table[i][index].bsize;
		        }

			}
			return min_est;
		}

		uint32_t getCardinality() {
    
	    	uint32_t card =0;

		    for (int i = 0; i < cm.depth; i++){
		        uint32_t countZero=0;
		        for (int j = 0; j < cm.width; j++){
		            if(cm.cm_table[i][j].bsize == 0){
		                countZero++;
		              
		            }
		        } 
		        card  += (uint32_t)(-cm.width*std::log(countZero*1.0/cm.width));      
		    }
	    
	    	return uint32_t(card/4);
	}

};

#endif