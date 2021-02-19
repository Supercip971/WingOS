#ifndef ETHERNET_PROTOCOL_H
#define ETHERNET_PROTOCOL_H
#include <stdint.h>

struct mac_address
{
    uint8_t mac[6];
} __attribute__((packed));

enum ethernet_types : uint16_t
{
    ET_ARP = 0x0806,
    ET_IP4 = 0x0800,
    ET_IP6 = 0x86DD,
};

struct ethernet_protocol
{
    mac_address destination;
    mac_address source;
    uint16_t ethernet_type;
    uint8_t payload[];
};

#endif // ETHERNET_PROTOCOL_H
