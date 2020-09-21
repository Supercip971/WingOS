#pragma once
#include <stdint.h>
enum memory_service_protocol_request
{
    REQUEST_MALLOC = 0,
    REQUEST_FREE = 1,
    REQUEST_REALLOC = 2
};

struct memory_service_protocol
{
    uint8_t request_type;
    uint64_t address;
    uint64_t length;
} __attribute__((packed));

void memory_service();
