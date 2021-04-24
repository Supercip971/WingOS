#pragma once
#include <64bit.h>
#include <stddef.h>
#include <utils/config.h>
#include <utils/lock.h>
#include <utils/attribute.h>
#include <virtual.h>
typedef page_table arch_page_table;
typedef uint64_t backtrace_entry_type;
#define arch_stackframe InterruptStackFrame

typedef void (*func)();
//int map_page(uint64_t phys_addr, uint64_t virt_addr, uint64_t flags);
#define arch_map_page(phys, virt, flags) map_page(phys, virt, flags)
#define arch_map_page_tbl(table, phys, virt, flags) map_page(table, phys, virt, flags)

#define PAGE_SIZE 4096
#define PAGE_ALIGN __attribute__((aligned(PAGE_SIZE)));

struct arch_process_data
{
    uint8_t *stack;
    uint64_t rsp = 0;
    uint64_t sse_context[128] __attribute__((aligned(64)));

    arch_page_table *page_directory = 0;
};

inline void halt_interrupt()
{
    asm volatile("hlt");
}
inline void turn_on_interrupt()
{
    asm volatile("sti");
}
inline void turn_off_interrupt()
{
    asm volatile("cli");
}

template <class T = uintptr_t, class F>
inline constexpr T get_usr_addr(F addr) { return reinterpret_cast<T>((uintptr_t)(addr) + USR_ADDR); }
template <class T = uintptr_t, class F>
inline constexpr T get_rusr_addr(F addr) { return reinterpret_cast<T>((uintptr_t)(addr)-USR_ADDR); }
template <class T = uintptr_t, class F>
inline constexpr T get_mem_addr(F addr) { return reinterpret_cast<T>((uintptr_t)(addr) + MEM_ADDR); }
template <class T = uintptr_t, class F>
inline constexpr T get_rmem_addr(F addr) { return reinterpret_cast<T>((uintptr_t)(addr)-MEM_ADDR); }
template <class T = uintptr_t, class F>
inline constexpr T get_kern_addr(F addr) { return reinterpret_cast<T>((uintptr_t)(addr) + KER_ADDR); }
template <class T = uintptr_t, class F>
inline constexpr T get_rkern_addr(F addr) { return reinterpret_cast<T>((uintptr_t)(addr)-KER_ADDR); }

size_t get_cpu_count();
size_t get_current_cpu_id();

void dump_stackframe(void *rbp);
