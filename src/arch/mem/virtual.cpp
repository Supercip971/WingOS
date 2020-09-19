#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/physical.h>
#include <arch/mem/virtual.h>
#include <com.h>
#include <device/acpi.h>
#include <kernel.h>
#include <stivale_struct.h>

#include <utility.h>
static struct pagemap kernel_super_dir;
struct pagemap *kernel_pagemap = &kernel_super_dir;

int map_page(uint64_t phys_addr, uint64_t virt_addr, uint64_t flags)
{
    uint64_t pml4_entry = (virt_addr & ((uint64_t)0x1ff << 39)) >> 39;
    uint64_t pdpt_entry = (virt_addr & ((uint64_t)0x1ff << 30)) >> 30;
    uint64_t pd_entry = (virt_addr & ((uint64_t)0x1ff << 21)) >> 21;
    uint64_t pt_entry = (virt_addr & ((uint64_t)0x1ff << 12)) >> 12;

    main_page_table *pdpt, *pd, *pt;

    if (kernel_pagemap->pml4[pml4_entry] & 0x1)
    {
        pdpt = (uint64_t *)(get_mem_addr(kernel_pagemap->pml4[pml4_entry] & 0xfffffffffffff000));
    }
    else
    {
        pdpt = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        kernel_pagemap->pml4[pml4_entry] = (uint64_t)(get_rmem_addr((uint64_t)pdpt)) | 0b111;
    }

    if (pdpt[pdpt_entry] & 0x1)
    {
        pd = (uint64_t *)(get_mem_addr(pdpt[pdpt_entry] & 0xfffffffffffff000));
    }
    else
    {
        pd = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pdpt[pdpt_entry] = (uint64_t)get_rmem_addr((uint64_t)pd) | 0b111;
    }

    if (pd[pd_entry] & 0x1)
    {
        pt = (uint64_t *)(get_mem_addr(pd[pd_entry] & 0xfffffffffffff000));
    }
    else
    {
        pt = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pd[pd_entry] = (uint64_t)get_rmem_addr((uint64_t)pt) | 0b111;
    }

    pt[pt_entry] = (uint64_t)(phys_addr | flags);

    return 0;
}

void init_vmm(stivale_struct *bootdata)
{
    kernel_pagemap->pml4 = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
    if ((uint64_t)kernel_pagemap->pml4 == MEM_PHYS_OFFSET)
    {
        return;
    }
    e820_entry_t *mementry = (e820_entry_t *)bootdata->memory_map_addr;

    for (uint64_t i = 0; i < (0x2000000 / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        map_page(addr, addr, 0x03);
        map_page(addr, get_mem_addr(addr), 0x03);
        map_page(addr, get_kern_addr(addr), 0x03 | (1 << 8));
    }

    set_paging_dir((uint64_t)kernel_pagemap->pml4 - MEM_PHYS_OFFSET);
    for (uint64_t i = 0; i < (0x100000000 / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;

        map_page(addr, get_mem_addr(addr), 0x03);
    }

    for (uint64_t i = 0; i < bootdata->memory_map_entries; i++)
    {

        uint64_t aligned_base = mementry[i].base - (mementry[i].base % PAGE_SIZE);
        uint64_t aligned_length = (mementry[i].length / PAGE_SIZE) * PAGE_SIZE;

        if (mementry[i].base % PAGE_SIZE)
            aligned_length += PAGE_SIZE;

        for (uint64_t j = 0; j * PAGE_SIZE < aligned_length; j++)
        {
            uint64_t addr = aligned_base + j * PAGE_SIZE;
            if (addr < 0x100000000)
                continue;

            map_page(addr, get_mem_addr(addr), 0x03);
        }
    }

    set_paging_dir((uint64_t)kernel_pagemap->pml4 - MEM_PHYS_OFFSET);
}
