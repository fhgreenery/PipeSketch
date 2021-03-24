#ifndef PIPE_H
#define PIPE_H
#include <unordered_set>
#include <utility>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "datatypes.hpp"
#include "hash.c"
#include <vector>
extern "C"
{
#include "hash.h"
#include "util.h"
}

class PipeSketch {

    typedef struct LBUCKET_type {
        uint16_t lg_bsize;
    } LBucket;

   struct Light_type {
        int lgwidth;
        int lgdepth = 4;
        uint64_t lgseed[4] = {541,1211,1862,1094};
        LBucket* light_table[4];
    };

    typedef struct RBUCKET_type {
        uint32_t rg_bsize;
    } RBucket;

    struct Regular_type {
        int rgwidth;
        int rgdepth = 3;
        uint64_t rgseed[3]={120,357,862};
        RBucket* regular_table[3];
    };

    typedef struct HBUCKET_type {
        unsigned char key[13];
        uint32_t hv_bsize;
        float  score;
    } HBucket;

    struct Heavy_type { //heavy party只一个table
        int hvwidth;
        uint64_t hvseed = 61;
        HBucket* heavy_table;
    };

 
    public:
        PipeSketch(int lgwidth,int rgwidth,int hvwidth);

        ~PipeSketch();

        void reset();

        void insertLight(unsigned char* key, int32_t pkt_size, uint16_t minlg);

        void insertRegular(unsigned char* key, int32_t pkt_size);

        void insertHeavy(unsigned char* key, int32_t pkt_size);

        void insert(unsigned char* key, int32_t pkt_size);

        uint32_t queryLight(unsigned char* key);

        uint32_t queryRegular(unsigned char* key);

        uint32_t queryHeavy(unsigned char* key);

        uint32_t query(unsigned char* key);

        uint32_t getCardinality(); 

        void get_distribution(std::vector<double> &dist);

        void get_entropy(double &entropy);

        void getHeavyHitter(val_tp thresh, myvector& results);


    private:
        Light_type lg;
        Regular_type rg;
        Heavy_type hv;
};

#endif
