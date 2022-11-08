#pragma once 

#include <stdint.h>
#include "libcore/ds/array.hpp"
namespace arch::amd64 
{
    struct Gdtr 
    {
        uint16_t limit;
        uint64_t base;


        constexpr Gdtr(uint64_t base, uint16_t limit) : base(base), limit(limit) {}

        constexpr Gdtr() : base(0), limit(0) {}



        template<typename T>
        constexpr static Gdtr gdtr_from_ptr(T* ptr) {
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

        constexpr static GdtEntry make_entry( uint32_t base, uint32_t limit, uint8_t granularity, uint8_t flags )
        {

            return (GdtEntry) {
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

        constexpr Gdt() : _entries({
            GdtEntry::make_entry(0, 0, 0, 0), // null segment
            GdtEntry::make_entry(0, 0xFFFFFFFF, 0x9A, 0xCF), // code segment
            GdtEntry::make_entry(0, 0xFFFFFFFF, 0x92, 0xCF), // data segment
            GdtEntry::make_entry(0, 0xFFFFFFFF, 0xFA, 0xCF), // user code segment
            GdtEntry::make_entry(0, 0xFFFFFFFF, 0xF2, 0xCF), // user data segment
        }) {}
    } __attribute__((packed));
}