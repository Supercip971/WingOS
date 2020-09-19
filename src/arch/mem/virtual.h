#pragma once
#include <arch/arch.h>
#include <arch/mem/physical.h>
#include <loggging.h>
#include <stivale_struct.h>
#define KERNEL_PHYS_OFFSET ((uint64_t)0xffffffff80000000)
#define MEM_PHYS_OFFSET ((uint64_t)0xffff800000000000)

#define PML4_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 39)) >> 39
#define PDPT_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 30)) >> 30
#define PAGE_DIR_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 21)) >> 21
#define PAGE_TABLE_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 12)) >> 12
#define FRAME_ADDR 0xfffffffffffff000

#define PAGE_SIZE 4096
#define TWO_MEGS 0x2000000
#define FOUR_GIGS 0x100000000
#define BASIC_PAGE_FLAGS 0x03
#define PAGE_TABLE_FLAGS 0x07
#define BIT_PRESENT 0x1
extern "C" void set_paging();
typedef uint64_t main_page_table;

extern main_page_table *main_pml4;
inline void set_paging_dir(uint64_t pd)
{
    //log("paging", LOG_INFO) << "setting new paging dir : " << pd;
    asm volatile("mov cr3, %0" ::"r"(pd));
    //  log("paging", LOG_INFO) << "new paging dir loaded ! ";
}
inline void update_paging()
{
    set_paging_dir((uint64_t)main_pml4 - 0xffff800000000000);
}
int map_page(uint64_t phys_addr, uint64_t virt_addr, uint64_t flags);

inline void virt_map(uint64_t from, uint64_t to, uint64_t flags)
{
    log("paging", LOG_ERROR) << "HEY BUDDY ! REPLACE VIRT MAP WITH MAP PAGE !!!!!";
    map_page(from, to, flags);
}
void init_vmm(stivale_struct *bootdata);
