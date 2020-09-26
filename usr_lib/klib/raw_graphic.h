#pragma once
#include <stdint.h>
namespace sys {
    enum graphic_buffert_service_request
    {
        GET_CURRENT_BUFFER_ADDR = 1,
    };
    struct graphic_buffer_protocol
    {
        uint8_t request;
        uint64_t data1; // for later (resolution / bpp)
        uint64_t data2;
        uint64_t data3;
    } __attribute__((packed));
    uint64_t get_graphic_buffer_addr();
}
