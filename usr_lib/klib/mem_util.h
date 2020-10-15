#pragma once
#include <stdint.h>
#include <stddef.h>
namespace sys {
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

    void* service_malloc(size_t length);
    void service_free(void* addr);
    void* service_realloc(void* addr, size_t length);
}
