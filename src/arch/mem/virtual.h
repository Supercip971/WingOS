#pragma once
#include <arch/arch.h>
#include <arch/mem/physical.h>
#include <stivale_struct.h>
#define PML4_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 39)) >> 39
#define PDPT_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 30)) >> 30
#define PAGE_DIR_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 21)) >> 21
#define PAGE_TABLE_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 12)) >> 12
#define FRAME_ADDR 0xfffffffffffff000

#define PAGE_SIZE 4096
#define TWO_MEGS 0x2000000
#define FOUR_GIGS 0x100000000
#define BASIC_PAGE_FLAGS 0b111
#define BIT_PRESENT 0x1

typedef uint64_t pl4_paging;

static pl4_paging *pl4_table __attribute__((aligned(4096)));

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

inline uint64_t vmm_get_cr3()
{
    uint64_t ret;
    asm volatile("movq %%cr3, %0;"
                 : "=r"(ret));
    return ret;
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
