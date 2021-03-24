#ifndef LC_H
#define LC_H
#include "hash.h"


class LinearCounting{

	typedef struct BUCKET_type {
        int32_t bsize;
    } Bucket;

   struct LC_type {
        int width;
        uint64_t seed =541;
        Bucket* lc_table;//定义一个指针，用来存储二维数组
    };

    
	private:
		LC_type lc;


	public:
		LinearCounting(int width){
			lc.width = width;
			lc.lc_table = new Bucket[width];
			memset(lc.lc_table, 0, width*sizeof(Bucket));
		}

		~LinearCounting(){
        	delete lc.lc_table;
			
		}
		void insert(unsigned char* key, int32_t pkt_size){

			int index = 0;
	        index = MurmurHash64A(key, 13, lc.seed)% lc.width;	       
	        lc.lc_table[index].bsize += pkt_size;
			
		}

		uint32_t getCardinality() {
    
	    	uint32_t card =0;
	        uint32_t countZero=0;
	        for (int j = 0; j < lc.width; j++){
	            if(lc.lc_table[j].bsize == 0){
	                countZero++;
	              
	            }
	        } 
	        card  = (uint32_t)(-lc.width*std::log(countZero*1.0/lc.width));      
	    
	    	return card;
	}

};

#endif