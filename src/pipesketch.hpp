#ifndef PIPE_H
#define PIPE_H
#include <vector>
#include <unordered_set>
#include <utility>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "BOBHash32.h"
#define LGN 13



class PipeSketch {

    typedef struct LBUCKET_type {
        int16_t lg_bsize;
    } LBucket;

   struct Light_type {
        int lgdepth;
        int lgwidth;
        LBucket **light_table;
    };

    typedef struct RBUCKET_type {
        int32_t rg_bsize;
    } RBucket;

    struct Regular_type {
        int rgdepth;
        int rgwidth;
        RBucket **regular_table;
    };

    typedef struct HBUCKET_type {
        char key[LGN];
        int32_t hv_bsize;
        float  score;
    } HBucket;

    struct Heavy_type {
        int hvdepth;
        int hvwidth;
        HBucket **heavy_table;
    };

 
    public:
        PipeSketch(int lgwidth,int lgdepth,int rgwidth,int rgdepth,int hvwidth,int hvdepth);

        ~PipeSketch();

        void insertLight(char* key, int32_t pkt_size, int32_t minlg);

        void insertRegular(char* key, int32_t pkt_size);

        void insertHeavy(char* key, int32_t pkt_size);

        void insert(char* key, int32_t pkt_size);


        int32_t queryLight(char* key);

        int32_t queryRegular(char* key);

        int32_t queryHeavy(char* key);

        int32_t query(char* key);

    private:
        Light_type lg;
        Regular_type rg;
        Heavy_type hv;
        BOBHash32* lghash[4];
        BOBHash32* rghash[3];
        BOBHash32* hvhash[1];
};

#endif
