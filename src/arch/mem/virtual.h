#pragma once
#include <arch/arch.h>
#include <stivale.h>
#define RMFLAGS 0x000FFFFFFFFFF000
#define PAGE_SIZE 4096
#define PML4_GET_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_GET_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PAGE_DIR_GET_INDEX(addr) (((addr) >> 21) & 0x1FF)
#define PAGE_TABLE_GET_INDEX(addr) (((addr) >> 12) & 0x1FF)
#define KERNEL_START_MEMORY 0xffffffff80000000
#define TO_PHYS_U64(ptr) ((uint64_t)ptr ^ (uint64_t)KERNEL_START_MEMORY)
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

#define PAGE_CPL0 0x83
#define PAGE_CPL3 0x87
typedef uint64_t pl4_paging;
#define PAGING_TABLE_COUNT 512
static pl4_paging* pl4_table __attribute__((aligned(4096)));
void init_virtual_memory(stivale_struct* sti_struct);

void free_frame(uint64_t ptr);

void* alloc_frame();

void* alloc_multiple_frame(uint64_t count, bool fast = false );
void* alloc_multiple_frame_zero(uint64_t count, bool fast = false );
void virt_map(uint64_t vaddress, uint64_t paddress, uint64_t flags );

inline void set_paging_dir(uint64_t pd){

    asm volatile ("mov %0, %%cr3":: "a"(pd): "memory");

}
