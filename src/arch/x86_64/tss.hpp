#pragma once

#include <stdint.h>

#include "libcore/ds/array.hpp"

namespace arch::amd64
{
struct Tss
{

    uint32_t _reserved0;
    uint64_t rsp[3];
    uint64_t _reserved1;

    uint64_t ist[7];
    uint32_t _reserved2;
    uint32_t _reserved3;
    uint16_t _reserved4;

    uint16_t iopb_offset;

    static constexpr Tss empty()
    {
        return (Tss){
            ._reserved0 = 0,
            .rsp = {0, 0, 0},
            ._reserved1 = 0,
            .ist = {0, 0, 0, 0, 0,
                    0, 0},
            ._reserved2 = 0,
            ._reserved3 = 0,
            ._reserved4 = 0,
            .iopb_offset = 0,
        };
    }

    static constexpr size_t size()
    {
        return sizeof(Tss);
    }

} __attribute__((packed));

} // namespace arch::amd64
