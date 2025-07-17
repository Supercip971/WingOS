#pragma once

#include <stdint.h>

#include "libcore/ds/array.hpp"
#include <arch/x86_64/tss.hpp>
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


struct TssEntry 
{
    uint16_t len; 
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t flags;
    uint8_t flags2;
    uint8_t base_high;
    uint32_t base_upper;
    uint32_t _reserved;

    TssEntry() : len(0), base_low(0), base_middle(0), flags(0), flags2(0), base_high(0), base_upper(0), _reserved(0) {}

    TssEntry(Tss* tss_ptr)
    {
        const uintptr_t tss_address = reinterpret_cast<uintptr_t>(tss_ptr);
        len = static_cast<uint16_t>(Tss::size() - 1);
        base_low = static_cast<uint16_t>(tss_address & 0xFFFF);
        base_middle = static_cast<uint8_t>((tss_address >> 16) &
                                            0xFF);
        flags = 0b10001001; 
        flags2 = 0b00000000; 
        base_high = static_cast<uint8_t>((tss_address >> 24) &
                                            0xFF);
        base_upper = static_cast<uint32_t>((tss_address >> 32) & 0xFFFFFFFF);
        _reserved = 0; // Reserved field
    }
} __attribute__((packed));

struct Gdt
{
    core::Array<GdtEntry, 5> _entries;

    static const int kernel_code_segment_id = 1;
    static const int kernel_data_segment_id = 2;
    static const int user_data_segment_id = 3;
    static const int user_code_segment_id = 4;
    static const int tss_segment_id = 5;

    Tss tss;

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


static constexpr auto segment_to_offset_kernel(uint16_t segment)
{
    return segment * 8;
}

static constexpr auto segment_to_offset_user(uint16_t segment)
{
    return (segment * 8) + 3;
}

} // namespace arch::amd64