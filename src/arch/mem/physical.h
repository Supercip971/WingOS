#pragma once
#include <arch/mem/virtual.h>
#include <stivale_struct.h>

void init_physical_memory(stivale_struct *bootdata);

uint64_t find_free_page();

void *pmm_alloc(uint64_t lenght);
void *pmm_alloc_fast(uint64_t lenght);
void *pmm_alloc_zero(uint64_t lenght);
void pmm_free(void *where, uint64_t lenght);
// higher half memory offset
inline uint64_t get_mem_addr(uint64_t addr) { return addr + 0xffff800000000000; }
inline uint64_t get_rmem_addr(uint64_t addr) { return addr - 0xffff800000000000; }
inline uint64_t get_kern_addr(uint64_t addr) { return addr + 0xffffffff80000000; }
inline uint64_t get_rkern_addr(uint64_t addr) { return addr - 0xffffffff80000000; }
