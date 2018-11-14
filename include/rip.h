#ifndef _RIP_H
#define _RIP_H

#include "ipv4.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>

#define RIP_REQUEST 1
#define RIP_RESPONSE 2
#define RIP_VERSION 2

#define RIPv2_ENTRY_SIZE 20
#define RIPv2_HEADER_SIZE 4
#define RIPv2_MAX_ENTRIES 25
#define RIPv2_PACKET_SIZE RIPv2_HEADER_SIZE + RIPv2_MAX_ENTRIES * RIPv2_ENTRY_SIZE

#define RIPv2_UDP_PORT 520

#define IP_CONFIG_FILE "config.txt"
#define ROUTE_CONFIG_FILE "routetable.txt"

#define UDP_RCV_TIMEOUT -1

/*Estructura de una entrada RIPv2*/
typedef struct ripv2_entry {
    uint16_t addr_id;                                                           //2 bytes
    uint16_t route_tag;                                                         //2 bytes
    ipv4_addr_t ip_addr;
    ipv4_addr_t subnet_mask;
    ipv4_addr_t next_hop;
    uint32_t metric;
} ripv2_entry_t;
// 2*2+4*4 = 4+16= 20 bytes

/*Estructura de un paquete ripv2*/
typedef struct ripv2_msg {
    uint8_t command;
    uint8_t version;
    uint16_t zeroes;            // 4 bytes
    ripv2_entry_t entries[RIPv2_MAX_ENTRIES];
} ripv2_msg_t;

#endif /* _RIP_H */
