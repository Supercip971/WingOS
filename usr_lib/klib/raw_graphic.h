#pragma once
#include <stdint.h>
namespace sys {
    enum graphic_buffert_service_request
    {
        GET_CURRENT_BUFFER_ADDR = 1,
        GET_SCREEN_SIZE = 2, // data1 = bool(is_height)
    };
    struct graphic_buffer_protocol
    {
        uint8_t request;
        uint64_t data1; // for later (resolution / bpp)
        uint64_t data2;
        uint64_t data3;
    } __attribute__((packed));
    uint64_t get_graphic_buffer_addr();
    uint64_t get_screen_width();
    uint64_t get_screen_height();
    // raw pos is used for 'uint64_t' pos to raw pos

    struct real_pos{
        uint32_t x;
        uint32_t y;
    }__attribute__((packed));

    struct raw_pos{
        union{
            real_pos rpos;
            uint64_t pos;
        };
    }__attribute__((packed));
}
