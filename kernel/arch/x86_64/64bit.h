#pragma once
#include <logging.h>
#include <stdint.h>
#include <stivale_struct.h>

#define STACK_SIZE 65536

struct InterruptStackFrame
{
    uintptr_t r15;
    uintptr_t r14;
    uintptr_t r13;
    uintptr_t r12;
    uintptr_t r11;
    uintptr_t r10;
    uintptr_t r9;
    uintptr_t r8;
    uintptr_t rbp;
    uintptr_t rdi;
    uintptr_t rsi;
    uintptr_t rdx;
    uintptr_t rcx;
    uintptr_t rbx;
    uintptr_t rax;

    // Contains error code and interrupt number for exceptions
    // Contains syscall number for syscalls
    // Contains just the interrupt number otherwise
    uint64_t int_no;
    uint64_t error_code;
    // Interrupt stack frame
    uintptr_t rip;
    uintptr_t cs;
    uintptr_t rflags;
    uintptr_t rsp;
    uintptr_t ss;
} __attribute__((packed));

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
static inline uintptr_t x86_get_cr3(void)
{
    uintptr_t rv;

    __asm__ __volatile__("mov %%cr3, %0"
                         : "=r"(rv));
    return rv;
}

static inline void x86_set_cr3(uintptr_t cr3)
{
    asm volatile("mov %0, %%cr3"
                 :
                 : "a"(cr3)
                 : "memory");
}
inline static uintptr_t x86_rdmsr(uintptr_t msr)
{

    uint32_t low, high;
    asm volatile("rdmsr"
                 : "=a"(low), "=d"(high)
                 : "c"(msr));
    return ((uintptr_t)high << 32) | low;
}

inline static void x86_wrmsr(uintptr_t msr, uintptr_t value)
{

    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile("wrmsr"
                 :
                 : "c"(msr), "a"(low), "d"(high));
}

inline static void x86_wrxcr(uintptr_t value)
{
    uint32_t edx = (value >> 32);
    uint32_t eax = (uint32_t)value;
    asm volatile("xsetbv"
                 :
                 : "a"(eax), "d"(edx), "c"(0));
}

static inline int x86_cpuid(uint32_t target, uint32_t low_target,
                            uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{

    asm volatile("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "a"(target), "c"(low_target));
    return 1;
}
#define MSR_EFER_SCE 1
enum MSR_REGISTERS
{
    APIC = 0x1B,
    EFER = 0xC0000080,
    STAR = 0xC0000081,
    LSTAR = 0xC0000082,
    COMPAT_STAR = 0xC0000083,
    SYSCALL_FLAG_MASK = 0xC0000084,
    FS_BASE = 0xC0000100,
    GS_BASE = 0xC0000101,
    KERN_GS_BASE = 0xc0000102,
};
union wrmsr_star_register
{
    struct
    {

        uint32_t eip; // unused
        uint16_t kernel_segment;
        uint16_t user_segment;
    };
    uint64_t raw;
} __attribute__((packed));

#define POKE(addr) (*((volatile uintptr_t *)(addr)))

void dump_register(InterruptStackFrame *stck);
