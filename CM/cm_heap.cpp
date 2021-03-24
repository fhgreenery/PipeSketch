#ifndef CMH_H
#define CMH_H
#include "../hash.h"
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include "../cuckoo_hashing.h"
#include "../datatypes.hpp"
const int key_len = 13;
const int capacity = 800; //等同于heavy part中bucket数目
class CMH{

	typedef pair <uint8_t[key_len], int> KV;
    KV heap[capacity];
    int heap_element_num;
    cuckoo::CuckooHashing<key_len, int(capacity * 2)> ht;

	typedef struct BUCKET_type {
        int32_t bsize;
    } Bucket;

   struct CMH_type {
        int depth = 4;
        int width;
        uint64_t seed[4] = {541,1211,1862,1094};
        Bucket* cm_table[4];//定义一个指针，用来存储二维数组
    };

    // heap
    void heap_adjust_down(int i) { 
        while (i < heap_element_num / 2) {
            int l_child = 2 * i + 1;
            int r_child = 2 * i + 2;
            int larger_one = i;
            if (l_child < heap_element_num && heap[l_child].second < heap[larger_one].second) {
                larger_one = l_child;
            }
            if (r_child < heap_element_num && heap[r_child].second < heap[larger_one].second) {
                larger_one = r_child;
            }
            if (larger_one != i) {
                swap(heap[i], heap[larger_one]);
//                swap(ht[string((const char *)heap[i].first, key_len)], ht[string((const char *)heap[larger_one].first, key_len)]);
                swap(ht[heap[i].first], ht[heap[larger_one].first]);
                heap_adjust_down(larger_one);
            } else {
                break;
            }
        }
    }

    void heap_adjust_up(int i) {
        while (i > 1) {
            int parent = (i - 1) / 2;
            if (heap[parent].second <= heap[i].second) {
                break;
            }
            swap(heap[i], heap[parent]);
//            swap(ht[string((const char *)heap[i].first, key_len)], ht[string((const char *)heap[parent].first, key_len)]);
            swap(ht[heap[i].first], ht[heap[parent].first]);
            i = parent;
        }
    }

	private:
		CMH_type cm;


	public:
		CMH(int width){
			heap_element_num = 0;
			cm.width = width;
			for (int i = 0; i < cm.depth; i++){
				cm.cm_table[i] = new Bucket[width];
				memset(cm.cm_table[i], 0, width*sizeof(Bucket));
			}

			memset(heap, 0, sizeof(heap));
	        for (int i = 0; i < capacity; ++i) {
	            heap[i].second = 0;
	        }
		}

		~CMH(){
			for (int i = 0; i < cm.depth; i++)
        		delete cm.cm_table[i];
			
		}
		void insert(unsigned char* key, int32_t pkt_size){
			int tmin = 1 << 30, ans = tmin; 
			int index = 0;
			for(int i=0; i<cm.depth; i++){ 
		        index = MurmurHash64A(key, 13, cm.seed[i])% cm.width;	       
		        cm.cm_table[i][index].bsize += pkt_size;
		        int val = cm.cm_table[i][index].bsize ;
		        ans = std::min(val, ans);
			}
			 if (ht.find(key)) //key-flowID 字符串数组
	        {

	            heap[ht[key]].second++;
	            heap_adjust_down(ht[key]);
	        } else if (heap_element_num < capacity) {
	            memcpy(heap[heap_element_num].first, key, key_len);
	            heap[heap_element_num].second = ans;
	            ht[key] = heap_element_num++;

	            heap_adjust_up(heap_element_num - 1);
	        } else if (ans > heap[0].second) {
	            KV & kv = heap[0];
	            ht.erase(kv.first);//删除堆顶点的值
	            memcpy(kv.first, key, key_len);//将新数据作为顶点
	            kv.second = ans;
	            ht[key] = 0;
	            heap_adjust_down(0);
	        }
		}
		uint32_t query(unsigned char* key){
			int tmin = 1 << 30, ans = tmin;
			int index = 0;
			// uint32_t min_est = 2147483647; //2^31-1
			for(int i=0; i<cm.depth; i++){
		        index = MurmurHash64A(key, 13, cm.seed[i]) % cm.width;
		        // if(cm.cm_table[i][index].bsize < min_est){
		        // 	min_est = cm.cm_table[i][index].bsize;
		        // }
		        int val = cm.cm_table[i][index].bsize ;
		        ans = std::min(val, ans);

			}
			return ans;
		}
		void get_heavy_hitters(uint32_t threshold, std::vector<pair<key_tp, val_tp> >& ret)
	    {
	        ret.clear();
	        for (int i = 0; i < capacity; ++i) { //遍历堆,找到heavy hitters
	            if (heap[i].second >= threshold) {
	  
		            key_tp key;
		            memcpy(key.key, heap[i].first, keylen);
		            std::pair<key_tp, val_tp> node;
		            node.first = key;
		            node.second = heap[i].second;
		            ret.push_back(node);
	            }
	        }
	    }

};

#endif