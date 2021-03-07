#include "pipesketch.hpp"
#include <bitset>
#include "datatypes.hpp"
const int keysize = 13;//key bytes

PipeSketch::PipeSketch(int lgwidth,int lgdepth,int rgwidth,int rgdepth,int hvwidth,int hvdepth){ 

	lg.lgdepth = lgdepth;
	lg.lgwidth = lgwidth;
	lg.light_table = new LBucket *[lgdepth*lgwidth];

	rg.rgdepth = rgdepth;
	rg.rgwidth = rgwidth;
	rg.regular_table = new RBucket *[rgdepth*rgwidth];

	hv.hvdepth = hvdepth;
	hv.hvwidth = hvwidth;
	hv.heavy_table = new HBucket *[hvdepth*hvwidth];

	for (int i = 0; i < lgdepth*lgwidth; i++) {
        lg.light_table[i] = (LBucket*)calloc(1, sizeof(LBucket));
        memset(lg.light_table[i], 0, sizeof(LBucket));
    }

    for (int i = 0; i < rgdepth*rgwidth; i++) {
        rg.regular_table[i] = (RBucket*)calloc(1, sizeof(RBucket));
        memset(rg.regular_table[i], 0, sizeof(RBucket));
    }

    for (int i = 0; i < hvdepth*hvwidth; i++) {
        hv.heavy_table[i] = (HBucket*)calloc(1, sizeof(HBucket));
        memset(hv.heavy_table[i], 0, sizeof(HBucket));
        hv.heavy_table[i]->key[0] = '\0';
    }

    for (int i = 0; i < lgdepth; i++)
        lghash[i] = new BOBHash32(i + 500);

    for (int i = 0; i < rgdepth; i++)
        rghash[i] = new BOBHash32(i + 750);

    for (int i = 0; i < hvdepth; i++)
        hvhash[i] = new BOBHash32(i + 1000);
}

PipeSketch::~PipeSketch(){

	for (int i = 0; i < lg.lgdepth*lg.lgwidth; i++) {
        free(lg.light_table[i]);
    }

    for (int i = 0; i < rg.rgdepth*rg.rgwidth; i++) {
        free(rg.regular_table[i]);
    }

    for (int i = 0; i < hv.hvdepth*hv.hvwidth; i++) {
        free(hv.heavy_table[i]);
    }

}

void PipeSketch::insertLight(char* key, int32_t pkt_size, int32_t minlg){
	int lgbucket = 0,index = 0;
	for(int i=0; i<lg.lgdepth; i++){ 
		lgbucket = lghash[i]->run(key,keysize) % lg.lgwidth; 
        index = i*lg.lgwidth+lgbucket;
        if(lg.light_table[index]->lg_bsize == minlg){
        	lg.light_table[index]->lg_bsize += pkt_size;
        }else{
        	lg.light_table[index]->lg_bsize = lg.light_table[index]->lg_bsize > (minlg+pkt_size)?lg.light_table[index]->lg_bsize:(minlg+pkt_size);
        }
	}
}

void PipeSketch::insertRegular(char* key, int32_t pkt_size){ //又是无符号整数的原因

	int32_t minrg = queryRegular(key);
    // cout<<"inserted rg:"<<pkt_size<<"rg key:"<<bitset<8>(key[0])<<endl;
	int rgbucket=0,index=0;

	for(int i=0; i<rg.rgdepth; i++){
		rgbucket = rghash[i]->run(key,keysize) % rg.rgwidth; 
        index = i*rg.rgwidth+rgbucket;
        if(rg.regular_table[index]->rg_bsize == minrg){

        	rg.regular_table[index]->rg_bsize += pkt_size;
            // cout<<"minrg:"<<minrg<<"sum:"<<rg.regular_table[index]->rg_bsize<<endl;
        }else{
        	rg.regular_table[index]->rg_bsize = max(rg.regular_table[index]->rg_bsize,(minrg+pkt_size));
        }
	}

}

void PipeSketch::insertHeavy(char* key, int32_t pkt_size){
    // cout<<"heavy pkt:"<<pkt_size<<" hv key:"<<bitset<8>(key[0])<<endl;
	int hvbucket=0,index=0 ;
	for(int i=0; i<hv.hvdepth; i++){
		hvbucket = hvhash[i]->run(key,keysize) % hv.hvwidth; //
        index = i*hv.hvwidth+hvbucket;
        PipeSketch::HBucket *hvb = hv.heavy_table[index];
        // cout<<"for hvb->key:"<<bitset<8>(hvb->key[0])<<endl;
        if (hvb->key[0] == '\0') { //key为空
            // cout<<"hv 1"<<endl;
        	memcpy(hvb->key, key, keysize);
        	hvb->hv_bsize += pkt_size;
        	hvb->score += 0.066*pkt_size;

        }else if(memcmp(key,hvb->key, keysize) == 0){ 
            // cout<<"hv 2"<<endl;
        	hvb->hv_bsize += pkt_size;
        	hvb->score += 0.066*pkt_size;
        	
        }else{
            // cout<<"hv 3"<<endl;
        	int count = ceil(hvb->score/100);
        	if(count > 100){ count = 100;} //防止溢出
        	if (!(rand()%int(pow(1.03,count)))){
        		hvb->score -= 0.066*pkt_size;
        		if(hvb->score <= 0){  
                    // cout<<"hv_bsize："<<hvb->hv_bsize<<"hvb key:"<<bitset<8>(hvb->key[0])<<endl;
        			insertRegular(hvb->key,hvb->hv_bsize);
        		    int before = queryRegular(key);
                    memcpy(hvb->key, key, keysize);
        			hvb->hv_bsize = pkt_size+before;
        			hvb->score = 0.066*hvb->hv_bsize; 
                    // cout<<"insert before:"<<(-before)<<endl;                     
        			insertRegular(key,-before);//原因在于定义的无符号整数
        		}else{
        			insertRegular(key,pkt_size);
        		}
        	}else{
        		insertRegular(key,pkt_size);
        	}
        	
        }
    }

	
}

void PipeSketch::insert(char* key, int32_t pkt_size){

	int32_t minlg = queryLight(key);
    if(minlg <= 2000){
		insertLight(key,pkt_size,minlg);
	}else{
		insertHeavy(key,pkt_size);
	}
}

int32_t PipeSketch::queryLight(char* key){
	int lgbucket=0,index=0;
	int32_t lg_est = 2147483647; //2^31-1
	for(int i=0; i<lg.lgdepth; i++){
		lgbucket = lghash[i]->run(key,keysize) % lg.lgwidth; 
        index = i*lg.lgwidth+lgbucket;
        if(lg.light_table[index]->lg_bsize < lg_est){
        	lg_est = lg.light_table[index]->lg_bsize;
        }

	}
	return lg_est;

}

int32_t PipeSketch::queryRegular(char* key){
	int rgbucket=0,index=0;
	int32_t rg_est = 2147483647; 
	for(int i=0; i<rg.rgdepth; i++){
		rgbucket = rghash[i]->run(key,keysize) % rg.rgwidth; 
        index = i*rg.rgwidth+rgbucket;
        if(rg.regular_table[index]->rg_bsize < rg_est){
        	rg_est = rg.regular_table[index]->rg_bsize ;
        }
	}
	return rg_est;
	
}

int32_t PipeSketch::queryHeavy(char* key){
	int hvbucket=0,index=0;
	for(int i=0; i<hv.hvdepth; i++){
		hvbucket = hvhash[i]->run(key,keysize) % hv.hvwidth; 
		index = i*hv.hvwidth+hvbucket;
		if(memcmp(key, hv.heavy_table[index]->key, keysize) == 0){
			return hv.heavy_table[index]->hv_bsize;
		}else{ 
			return -1;
		}
	}
	
}

//
int32_t PipeSketch::query(char* key){

	int32_t min_lg = queryLight(key);
	if(min_lg <= 2000){
		return 20000; //min_lg
	}else{
		int32_t min_hv = queryHeavy(key);
		if(min_hv != -1){
			return min_lg;//min_hv+min_lg;
		} else{
			int32_t min_rg = queryRegular(key);
			return min_lg; //min_rg
		}
	}
	
}