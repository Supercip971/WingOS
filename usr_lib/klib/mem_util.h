#pragma once
#include <stddef.h>
#include <stdint.h>
namespace sys
{
    enum memory_service_protocol_request
    {
        REQUEST_MALLOC = 0,
        REQUEST_FREE = 1,
        REQUEST_REALLOC = 2,
        REQUEST_PMM_MALLOC = 3,
        REQUEST_PMM_FREE = 4
    };

    struct memory_service_protocol
    {
        uint8_t request_type;
        uint64_t address;
        uint64_t length;
    } __attribute__((packed));

    void *service_pmm_malloc(size_t length);
    void service_pmm_free(void *addr, size_t length);
} // namespace sys
