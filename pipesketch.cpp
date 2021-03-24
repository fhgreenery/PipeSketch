#ifndef PIPESKETCH_H
#define PIPESKETCH_H
#include<bitset>
#include "pipesketch.hpp"
#include "EMFSD.h"
const int keylen = 13;//key bytes

PipeSketch::PipeSketch(int lgwidth,int rgwidth,int hvwidth){ 

	lg.lgwidth = lgwidth;
    rg.rgwidth = rgwidth;
    hv.hvwidth = hvwidth;

    for (int i = 0; i < lg.lgdepth; i++) {
        lg.light_table[i] = new LBucket[lgwidth];
        memset(lg.light_table[i], 0, lgwidth*sizeof(LBucket));
    }
    
    for (int i = 0; i < rg.rgdepth; i++) {
        rg.regular_table[i] = new RBucket[rgwidth];
        memset(rg.regular_table[i], 0, rgwidth*sizeof(RBucket));
    }
    
    hv.heavy_table = new HBucket[hvwidth];
    memset(hv.heavy_table, 0, hvwidth*sizeof(HBucket));

    for(int i = 0; i < hvwidth; i++){
        hv.heavy_table[i].key[0] = '\0';
    }
	
}

PipeSketch::~PipeSketch(){ //使用起来
    //对应delete
	for (int i = 0; i < lg.lgdepth; i++) {
        delete lg.light_table[i];
    }

    for (int i = 0; i < rg.rgdepth; i++) {
        delete rg.regular_table[i];
    }

    delete hv.heavy_table;
    
}
void PipeSketch::reset(){

    for (int i = 0; i < lg.lgdepth; i++) {
        for (int j = 0; j < lg.lgwidth; j++){
            lg.light_table[i][j].lg_bsize = 0;
        }       
    }

    for (int i = 0; i < rg.rgdepth; i++) {
        for (int j = 0; j < rg.rgwidth; j++){
            rg.regular_table[i][j].rg_bsize = 0;
        }
    }

    for (int i = 0; i < hv.hvwidth; i++) {
        hv.heavy_table[i].hv_bsize = 0;
        hv.heavy_table[i].score = 0;
        memset(hv.heavy_table[i].key, 0, keylen);
    }
}
void PipeSketch::getHeavyHitter(val_tp thresh, myvector& results){

    //只在heavy_table中找 
    for(int i=0; i<hv.hvwidth; i++){
        uint16_t minlg = queryLight((unsigned char*)&(hv.heavy_table[i].key));
        if((hv.heavy_table[i].hv_bsize+minlg) > thresh){
            key_tp key;
            memcpy(key.key, hv.heavy_table[i].key, keylen);
            std::pair<key_tp, val_tp> node;
            node.first = key;
            node.second = hv.heavy_table[i].hv_bsize+minlg;
            results.push_back(node);
        }

    }
}

uint32_t PipeSketch::getCardinality() {
    
    uint32_t card =0;

    for (int i = 0; i < lg.lgdepth; i++) {
        uint32_t countZero=0;
        for (int j = 0; j < lg.lgwidth; j++){
            if(lg.light_table[i][j].lg_bsize == 0){
                countZero++;
                
            }
        }
        std::cout<<"countZero:"<<countZero<<std::endl;
        card  += (uint32_t)(-lg.lgwidth*std::log(countZero*1.0/lg.lgwidth));      
    }
    
    return uint32_t(card/4);
}

void PipeSketch::get_entropy(double &entropy){
    double entr ;
    vector<double> dist;
    get_distribution(dist);
    for(int i = 0; i < (int)dist.size(); ++i){
        entr += dist[i]*log2(dist[i]);
    }
    uint32_t m = getCardinality();
    entropy = log(m)-(1/m)*entr;
}

void PipeSketch::get_distribution(vector<double> &dist){   
    EMFSD *em_fsd_algo = NULL;
    int counter_num = lg.lgwidth;
    uint32_t tmp_counters[counter_num];
    for (int i = 0; i < counter_num; i++)
        tmp_counters[i] = lg.light_table[0][i].lg_bsize;//根据第一个hash table进行计数

    em_fsd_algo = new EMFSD();
    em_fsd_algo->set_counters(counter_num, tmp_counters);
    std::cout<<"finish set_counters"<<std::endl;
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();
    em_fsd_algo->next_epoch();

    dist = em_fsd_algo->ns;//round(em_fsd_algo->ns) //四舍五入

}

void PipeSketch::insertLight(unsigned char* key, int32_t pkt_size, uint16_t minlg){
	// int index = 0;
	for(int i=0; i<lg.lgdepth; i++){ 
        int index = MurmurHash64A(key, keylen, lg.lgseed[i])% lg.lgwidth;
        if(lg.light_table[i][index].lg_bsize == minlg){
        	lg.light_table[i][index].lg_bsize += pkt_size;
        }else{
        	lg.light_table[i][index].lg_bsize = std::max(lg.light_table[i][index].lg_bsize,(uint16_t)(minlg+pkt_size));
        }
	}
}

void PipeSketch::insertRegular(unsigned char* key, int32_t pkt_size){ //又是无符号整数的原因

	uint32_t minrg = queryRegular(key);
	// int index=0;
	for(int i=0; i<rg.rgdepth; i++){
        int index = MurmurHash64A(key, keylen, rg.rgseed[i]) % rg.rgwidth;
        if(rg.regular_table[i][index].rg_bsize == minrg){
        	rg.regular_table[i][index].rg_bsize += pkt_size;
        }else{
        	rg.regular_table[i][index].rg_bsize = std::max(rg.regular_table[i][index].rg_bsize,(minrg+pkt_size));
        }
	}
}

void PipeSketch::insertHeavy(unsigned char* key, int32_t pkt_size){
	int index = MurmurHash64A(key, keylen, hv.hvseed) % hv.hvwidth;
    PipeSketch::HBucket *hvb = &hv.heavy_table[index]; //地址不变，但内容在变
        if (hvb->key[0] == '\0') { //key为空
            memcpy(hvb->key, key, keylen);
            hvb->hv_bsize += pkt_size;
            hvb->score += 0.066*pkt_size;

        }else if(memcmp(key,hvb->key, keylen) == 0){
            hvb->hv_bsize += pkt_size;
            hvb->score += 0.066*pkt_size;
            
        }else{
            int count = ceil(hvb->score/100);
            if(count > 100){ count = 100;} //防止溢出
            if (!(rand()%int(pow(1.03,count)))){
                hvb->score -= 0.066*pkt_size;
                if(hvb->score <= 0){
                    // cout<<"hv_bsize："<<hvb->hv_bsize<<"hvb key:"<<bitset<8>(hvb->key[0])<<endl;
                    insertRegular((unsigned char*)&(hvb->key),hvb->hv_bsize);
                    int before = queryRegular(key);
                    memcpy(hvb->key, key, keylen);
                    hvb->hv_bsize = pkt_size+before;
                    hvb->score = 0.066*hvb->hv_bsize;
                    insertRegular(key,-before);//原因在于定义的无符号整数
                }else{
                    insertRegular(key,pkt_size);
                }
            }else{
                insertRegular(key,pkt_size);
            }
            
        }

}

void PipeSketch::insert(unsigned char* key, int32_t pkt_size){
	uint16_t minlg = queryLight(key);
    if(minlg <= 64035){//65535-1500
		insertLight(key,pkt_size,minlg);
	}else{
		insertHeavy(key,pkt_size);
	}
}

uint32_t PipeSketch::queryLight(unsigned char* key){
	// int index=0;
	uint32_t lg_est = 4294967295; //2^31-1
	for(int i=0; i<lg.lgdepth; i++){
        int index = MurmurHash64A(key, keylen, lg.lgseed[i]) % lg.lgwidth;
        if(lg.light_table[i][index].lg_bsize < lg_est){
        	lg_est = lg.light_table[i][index].lg_bsize;
        }

	}
	return lg_est;

}

uint32_t PipeSketch::queryRegular(unsigned char* key){
	// int index=0;
	uint32_t rg_est = 4294967295; 
	for(int i=0; i<rg.rgdepth; i++){
        int index = MurmurHash64A(key, keylen, rg.rgseed[i]) % rg.rgwidth;
        if(rg.regular_table[i][index].rg_bsize < rg_est){
        	rg_est = rg.regular_table[i][index].rg_bsize ;
        }
	}
	return rg_est;
	
}

uint32_t PipeSketch::queryHeavy(unsigned char* key){
    int index = MurmurHash64A(key, keylen, hv.hvseed) % hv.hvwidth;
	if(memcmp(key, hv.heavy_table[index].key, keylen) == 0){
		return hv.heavy_table[index].hv_bsize;

	}else{ 
		return 4294967295;
	}
	
}

//
uint32_t PipeSketch::query(unsigned char* key){

	uint16_t min_lg = queryLight(key);
	if(min_lg <= 64035){
		return min_lg; //
	}else{
		uint32_t min_hv = queryHeavy(key);
		if(min_hv != 4294967295){
			return min_hv+min_lg;;//
		} else{
			uint32_t min_rg = queryRegular(key);
			return min_rg+min_lg; //min_lg
		}
	}
	
}
#endif