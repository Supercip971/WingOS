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

typedef uint32_t ip_addr;
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
struct address_resolution_protocol
{
    uint16_t link_layer_type;
    uint16_t protocol_type;
    uint8_t link_layer_size;
    uint8_t protocol_size;
    uint16_t message;
    uint8_t data[];
} __attribute__((packed));

struct ip_header
{
    uint8_t version : 4;
    uint8_t internet_header_length : 4;
    uint8_t type_of_service;
    uint16_t length;
    uint16_t id;
    uint16_t flags : 3;
    uint16_t fragment_offset : 13;
    uint8_t time_to_live;
    uint8_t proto;
    uint16_t check_sum;
    ip_addr source_addr;
    ip_addr target_addr;
} __attribute__((packed));

struct arp_ipv4_header
{
    mac_address sender;
    ip_addr sender_ip;
    mac_address receiver;
    ip_addr receiver_ip;
} __attribute__((packed));

struct udp_header
{
    uint16_t source_port;
    uint16_t target_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

struct icmp_v4
{
    uint8_t type;
    uint8_t code;
    uint16_t check_sum;
    uint8_t data[];
} __attribute__((packed));

#endif // ETHERNET_PROTOCOL_H
