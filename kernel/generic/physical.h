#pragma once
#include <arch.h>
#include <stivale_struct.h>
#include <utils/bitmap.h>
extern uint64_t available_memory;
void init_physical_memory(stivale2_struct_tag_memmap *bootdata);

uint64_t find_free_page();
uint64_t get_used_memory();
uint64_t get_total_memory();
void *pmm_alloc(uint64_t lenght);
void *pmm_alloc_zero(uint64_t lenght);
void pmm_free(void *where, uint64_t lenght);
// higher half memory offset

#define ALIGN_UP(addr, size) \
    ((addr % size == 0) ? (addr) : (addr) + size - ((addr) % size))

#define ALIGN_DOWN(addr, size) ((addr) - ((addr) % size))
