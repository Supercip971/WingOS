#pragma once
#include <logging.h>
#include <physical.h>
#include <stivale_struct.h>
#define KERNEL_PHYS_OFFSET ((uint64_t)0xffffffff80000000)
#define MEM_PHYS_OFFSET ((uint64_t)0xffff800000000000)
#define PML4_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 39)) >> 39
#define PDPT_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 30)) >> 30
#define PAGE_DIR_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 21)) >> 21
#define PAGE_TABLE_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 12)) >> 12
#define FRAME_ADDR 0xfffffffffffff000

#define PAGE_SIZE 4096
#define TWO_MEGS (0x2000000)
#define FOUR_GIGS 0x100000000
#define BASIC_PAGE_FLAGS 0x03
#define PAGE_TABLE_FLAGS 0x07
#define BIT_PRESENT 0x1
ASM_FUNCTION void set_paging();
typedef uint64_t main_page_table;

extern main_page_table *kernel_super_dir;
extern main_page_table *main_pml4;
inline void set_paging_dir(uint64_t pd)
{
    asm volatile("mov cr3, %0" ::"r"(pd & FRAME_ADDR));
}
void update_paging();
int map_page(uint64_t phys_addr, uint64_t virt_addr, uint64_t flags);
int map_page(main_page_table *table, uint64_t phys_addr, uint64_t virt_addr, uint64_t flags);
uint64_t get_physical_addr(uint64_t virt);
main_page_table *new_vmm_page_dir();
inline void virt_map(uint64_t from, uint64_t to, uint64_t flags)
{
    map_page(from, to, flags);
}
void init_vmm(stivale_struct *bootdata);

uintptr_t alloc_vmm_page(size_t length);
