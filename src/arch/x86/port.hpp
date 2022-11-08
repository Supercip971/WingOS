#pragma once 

#include <stdint.h>


namespace arch::x86 
{
    static inline void out8(uint16_t port, uint8_t value) 
    {
        asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
    }

    static inline void out16(uint16_t port, uint16_t value) 
    {
        asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
    }

    static inline void out32(uint16_t port, uint32_t value) 
    {
        asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
    }

    static inline uint8_t in8(uint16_t port) 
    {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }


    static inline uint16_t in16(uint16_t port) 
    {
        uint16_t ret;
        asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }


    static inline uint32_t in32(uint16_t port) 
    {
        uint32_t ret;
        asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }





}