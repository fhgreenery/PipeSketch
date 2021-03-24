#ifndef CUSKETCH_H
#define CUSKETCH_H
#include "../hash.h"

class CUSketch{

	typedef struct BUCKET_type {
        int32_t bsize;
    } Bucket;

   struct CU_type {
        int width;
        int depth = 4;
        uint64_t seed[4] = {541,1211,1862,1094};
        Bucket* cu_table[4];
    };
	private:
		CU_type cu;


	public:
		CUSketch(int width){
			cu.width = width;
			for (int i = 0; i < cu.depth; i++){
		        cu.cu_table[i] = new Bucket[width];
		        memset(cu.cu_table[i], 0, width*sizeof(Bucket));
	    	}
		}

		~CUSketch(){
			for (int i = 0; i < cu.depth; i++)
        		delete cu.cu_table[i];
			
		}
		void insert(unsigned char* key, int32_t pkt_size){
			int32_t min_lg = query(key);
			int index = 0;
			for(int i=0; i<cu.depth; i++){ 
		        index = MurmurHash64A(key, 13, cu.seed[i])% cu.width;
		        if(cu.cu_table[i][index].bsize == min_lg){
		        	cu.cu_table[i][index].bsize += pkt_size;
		        }else{
		        	cu.cu_table[i][index].bsize = std::max(cu.cu_table[i][index].bsize,(pkt_size+min_lg));
		        }	       
		        
			}


		}
		uint32_t query(unsigned char* key){
			int index = 0;
			uint32_t min_est = 2147483647; //2^31-1
			for(int i=0; i<cu.depth; i++){
		        index = MurmurHash64A(key, 13, cu.seed[i]) % cu.width;
		        if(cu.cu_table[i][index].bsize < min_est){
		        	min_est = cu.cu_table[i][index].bsize;
		        }

			}
			return min_est;
		}

};

#endif