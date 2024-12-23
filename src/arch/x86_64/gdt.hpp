#pragma once

#include <stdint.h>

#include "libcore/ds/array.hpp"

namespace arch::amd64
{
struct Gdtr
{
    uint16_t limit;
    uint64_t base;

    constexpr Gdtr(uint64_t base, uint16_t limit) : limit(limit), base(base) {}

    constexpr Gdtr() : limit(0), base(0) {}

    template <typename T>
    constexpr static Gdtr gdtr_from_ptr(T *ptr)
    {
        return Gdtr((uint64_t)ptr, sizeof(T) - 1);
    }

} __attribute__((packed));

struct GdtEntry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t flags;
    uint8_t limit_high : 4;
    uint8_t granularity : 4;
    uint8_t base_high;

    constexpr static GdtEntry make_entry(uint32_t base, uint32_t limit, uint8_t granularity, uint8_t flags)
    {
        return (GdtEntry){
            .limit_low = static_cast<uint16_t>(limit & 0xFFFF),
            .base_low = static_cast<uint16_t>(base & 0xFFFF),
            .base_middle = static_cast<uint8_t>((base >> 16) & 0xFF),
            .flags = flags,
            .limit_high = static_cast<uint8_t>((limit >> 16) & 0x0F),
            .granularity = static_cast<uint8_t>(granularity),
            .base_high = static_cast<uint8_t>((base >> 24) & 0xFF),
        };
    };
} __attribute__((packed));

struct Gdt
{
    core::Array<GdtEntry, 5> _entries;

    static const int kernel_code_segment_id = 1;
    static const int kernel_data_segment_id = 2;
    static const int user_data_segment_id = 3;
    static const int user_code_segment_id = 4;

    constexpr Gdt() : _entries({
                          GdtEntry::make_entry(0, 0, 0, 0),                      // null segment
                          GdtEntry::make_entry(0, 0xFFFFFFFF, 0b10, 0b10011010), // code segment
                          GdtEntry::make_entry(0, 0xFFFFFFFF, 0, 0b10010010),    // data segment
                          GdtEntry::make_entry(0, 0xFFFFFFFF, 0, 0b11110010),    // user data segment
                          GdtEntry::make_entry(0, 0xFFFFFFFF, 0b10, 0b11111010), // user code segment
                      }) {};

} __attribute__((packed));

Gdtr *load_default_gdt();
void gdt_use(Gdtr *gdtr);

} // namespace arch::amd64