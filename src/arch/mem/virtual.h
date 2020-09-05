#pragma once
#include <arch/arch.h>
#include <arch/mem/physical.h>
#include <stivale_struct.h>
#define RMFLAGS 0x000FFFFFFFFFF000
#define PAGE_SIZE 4096
#define PML4_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 39)) >> 39
#define PDPT_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 30)) >> 30
#define PAGE_DIR_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 21)) >> 21
#define PAGE_TABLE_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 12)) >> 12
#define KERNEL_START_MEMORY 0xffffffff80000000
#define TO_PHYS_U64(ptr) ((uint64_t)ptr ^ (uint64_t)KERNEL_START_MEMORY)
#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

#define PAGE_CPL0 0x83
#define PAGE_CPL3 0x87
typedef uint64_t pl4_paging;
#define PAGING_TABLE_COUNT 512
static pl4_paging *pl4_table __attribute__((aligned(4096)));
static uint64_t default_paging = 0x0;
void init_virtual_memory(stivale_struct *sti_struct);

uint64_t get_mem_addr(uint64_t addr);
uint64_t get_rmem_addr(uint64_t addr);

void virt_map(uint64_t vaddress, uint64_t paddress, uint64_t flags);
void Huge_virt_map(uint64_t paddress, uint64_t vaddress, uint64_t flags);
void update_paging();
inline void set_paging_dir(uint64_t pd)
{
    asm volatile("mov %%rax, %%cr3" ::"a"(pd)
                 : "memory");
}
inline void map_mem_address(uint64_t address, uint64_t page_length, bool with_offset = true)
{
    if ((address / PAGE_SIZE) * PAGE_SIZE != address)
    {
        address /= PAGE_SIZE;
        address *= PAGE_SIZE;
    }

    if (with_offset)
    {
        for (uint64_t i = 0; i < page_length; i++)
        {
            virt_map(address + (i * 4096), get_mem_addr(address + (i * 4096)), 0x03);
        }
    }
    else
    {
        for (uint64_t i = 0; i < page_length; i++)
        {
            virt_map(address + (i * 4096), (address + (i * 4096)), 0x03);
        }
    }
    update_paging();
}
void Huge_virt_map(uint64_t paddress, uint64_t vaddress, uint64_t flags);
inline uint64_t vmm_get_cr3()
{
    uint64_t ret;
    asm volatile("movq %%cr3, %0;"
                 : "=r"(ret));
    return ret;
}
