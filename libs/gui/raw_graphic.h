#pragma once
#include <stdint.h>
namespace sys
{

    struct real_pos
    {
        uint32_t x;
        uint32_t y;
    } __attribute__((packed));

    struct raw_pos
    {
        union
        {
            real_pos rpos;
            uint64_t pos;
        };
    } __attribute__((packed));

} // namespace sys
