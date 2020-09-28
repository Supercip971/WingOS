#pragma once
#include <int_value.h>
#include <loggging.h>
#include <stdint.h>
#include <stivale_struct.h>
#define X64

#ifdef X64
#define STACK_SIZE 8192
static char stack[STACK_SIZE] = {0};
static uint64_t bootdat = 0;
extern stivale_struct boot_loader_data_copy;
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
inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile("in eax, dx"
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
    log("rdmsr", LOG_INFO) << "reading msr : " << msr;
    uint32_t low, high;
    asm volatile("rdmsr"
                 : "=a"(low), "=d"(high)
                 : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

inline static void x86_wrmsr(uint64_t msr, uint64_t value)
{
    log("rdmsr", LOG_INFO) << "writing msr : " << msr << " = " << value;
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile("wrmsr"
                 :
                 : "c"(msr), "a"(low), "d"(high));
}

#define POKE(addr) (*((volatile uint64_t *)(addr)))
#endif // DEBUG
