#ifndef __TOOLS_H__
#define __TOOLS_H__

#include<string.h>
typedef struct __attribute__ ((__packed__)) five_tuple_t {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
} five_tuple;

typedef struct __attribute__ ((__packed__)) pkt_t {
    five_tuple key;
    int32_t pkt_size;
} pkt;

#endif