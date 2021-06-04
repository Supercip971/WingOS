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
} __attribute__((packed));

enum arp_commands : uint16_t
{
    ARP_REQUEST = 1,
    ARP_REPLY = 2,
    RARP_REQUEST = 3,
    RARP_REPLY = 4,
};
struct Address_resolution_protocol
{
    uint16_t link_layer_type;
    uint16_t protocol_type;
    uint8_t link_layer_size;
    uint8_t protocol_size;
    uint16_t message;
    uint8_t data[];
};

struct arp_ipv4_header
{
    mac_address sender;
    uint32_t sender_ip;
    mac_address receiver;
    uint32_t receiver_ip;
};

#endif // ETHERNET_PROTOCOL_H
