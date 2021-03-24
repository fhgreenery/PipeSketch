#ifndef CHANGER_H
#define CHANGER_H

#include "pipesketch.hpp"
#include "pipesketch.cpp"
#include "datatypes.hpp"
#include <cmath>
template <class S>
class HeavyChanger {

public:
    HeavyChanger(int lgwidth,int lgdepth,int rgwidth,int rgdepth,int hvwidth,int hvdepth);

    ~HeavyChanger();

    void insert(unsigned char* key, int32_t pkt_size) ;

    void query(val_tp thresh, myvector& result);

    void Reset();

    S* GetCurSketch();

    S* GetOldSketch();

private:
    S* old_sk;

    S* cur_sk;

    int lgn_;
};

template <class S>
HeavyChanger<S>::HeavyChanger(int lgwidth,int lgdepth,int rgwidth,int rgdepth,int hvwidth,int hvdepth) {
    old_sk = new S(lgwidth,lgdepth,rgwidth,rgdepth,hvwidth,hvdepth);
    cur_sk = new S(lgwidth,lgdepth,rgwidth,rgdepth,hvwidth,hvdepth);
    // lgn_ = lgn;
}


template <class S>
HeavyChanger<S>::~HeavyChanger() {
    delete old_sk;
    delete cur_sk;
}

template <class S>
void HeavyChanger<S>::insert(unsigned char* key, int32_t pkt_size) {
    cur_sk->insert(key, pkt_size);
}

template <class S>
void HeavyChanger<S>::query(val_tp thresh, myvector& result) {
    myvector res1, res2;

    cur_sk->getHeavyHitter(thresh, res1);//超过阈值的
    old_sk->getHeavyHitter(thresh, res2);
    myset reset; //set中不含重复数据，求交集
    for (auto it = res1.begin(); it != res1.end(); it++) {
        reset.insert(it->first);
    }
    for (auto it = res2.begin(); it != res2.end(); it++) {
        reset.insert(it->first);
    }
    // val_tp old_low, old_up;
    // val_tp new_low, new_up;
    // val_tp diff1, diff2;
    val_tp old_value, new_value;
    val_tp change;
    for (auto it = reset.begin(); it != reset.end(); it++) {
        old_value = old_sk->query((unsigned char*)(*it).key);
        new_value = cur_sk->query((unsigned char*)(*it).key);
        change = std::abs((int)(old_value-new_value));

        if (change > thresh) {
            // std::cout<<"pipe heavy changer:"<<change<<std::endl;
            key_tp key;
            memcpy(key.key, it->key, 13);
            std::pair<key_tp, val_tp> cand;
            cand.first = key;
            cand.second = change;
            result.push_back(cand);
        }
    }
}


template <class S>
void HeavyChanger<S>::Reset() {
    old_sk->reset();
    S* temp = old_sk;
    old_sk = cur_sk;
    cur_sk = temp;
}

template <class S>
S* HeavyChanger<S>::GetCurSketch() {
    return cur_sk;
}

template <class S>
S* HeavyChanger<S>::GetOldSketch() {
    return old_sk;
}

#endif
