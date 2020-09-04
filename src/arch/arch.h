#pragma once
#include <int_value.h>
#include <stdint.h>
#define X64

#ifdef X64
#define STACK_SIZE 4096
inline void outb(uint16_t port, uint8_t value)
{
    asm volatile("out  dx, al" ::"a"(value), "d"(port));
}
inline void outw(uint16_t port, uint16_t value)
{
    asm volatile("out  dx, ax" ::"a"(value), "d"(port));
}
inline void outl(uint16_t port, uint32_t value)
{
    asm volatile("out  dx, eax" ::"a"(value), "d"(port));
}
inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("in al, dx"
                 : "=a"(ret)
                 : "d"(port));
    return ret;
}
inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile("in ax, dx"
                 : "=a"(ret)
                 : "d"(port));
    return ret;
}
inline void wait()
{
    for (int i = 0; i < 40; i++)
    {
        inw(i);
    }
}
static inline uint64_t x86_get_cr3(void)
{
    uint64_t rv;

    __asm__ __volatile__("mov %%cr3, %0"
                         : "=r"(rv));
    return rv;
}

static inline void x86_set_cr3(uint64_t cr3)
{
    asm volatile("mov %0, %%cr3"
                 :
                 : "a"(cr3)
                 : "memory");
}
inline static uint64_t x86_rdmsr(uint64_t msr)
{
    uint32_t low, high;
    asm volatile("rdmsr"
                 : "=a"(low), "=d"(high)
                 : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

inline static void x86_wrmsr(uint64_t msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile("wrmsr"
                 :
                 : "c"(msr), "a"(low), "d"(high));
}

#define POKE(addr) (*((volatile uint64_t *)(addr)))
#endif // DEBUG
