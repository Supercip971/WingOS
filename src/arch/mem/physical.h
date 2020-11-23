#pragma once
#include <arch/mem/virtual.h>
#include <stivale_struct.h>

extern uint64_t available_memory;
void init_physical_memory(stivale_struct *bootdata);

uint64_t find_free_page();
uint64_t get_used_memory();
uint64_t get_total_memory();
void *pmm_alloc(uint64_t lenght);
void *pmm_alloc_fast(uint64_t lenght);
void *pmm_alloc_zero(uint64_t lenght);
void pmm_free(void *where, uint64_t lenght);
// higher half memory offset

template <class T = uint64_t, class F>
inline constexpr T get_mem_addr(F addr) { return reinterpret_cast<T>(static_cast<uint64_t>(addr) + 0xffff800000000000); }
template <class T = uint64_t, class F>
inline constexpr T get_rmem_addr(F addr) { return reinterpret_cast<T>(static_cast<uint64_t>(addr) - 0xffff800000000000); }
template <class T = uint64_t, class F>
inline constexpr T get_kern_addr(F addr) { return reinterpret_cast<T>(static_cast<uint64_t>(addr) + 0xffffffff80000000); }
template <class T = uint64_t, class F>
inline constexpr T get_rkern_addr(F addr) { return reinterpret_cast<T>(static_cast<uint64_t>(addr) - 0xffffffff80000000); }
