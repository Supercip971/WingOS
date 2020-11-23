#include <arch/arch.h>

#include <arch/mem/memory_manager.h>
#include <arch/mem/physical.h>
#include <arch/mem/virtual.h>
#include <com.h>
#include <device/acpi.h>
#include <device/local_data.h>
#include <kernel.h>
#include <stivale_struct.h>

#include <utility.h>
main_page_table *kernel_super_dir;
main_page_table *main_pml4 = kernel_super_dir;
uint64_t entry_to_address(uint64_t pml4, uint64_t pdpt, uint64_t pd, uint64_t pt)
{
    uint64_t result = 0;

    result |= pml4 << 39;
    result |= pdpt << 30;
    result |= pd << 21;
    result |= pt << 12;

    return result;
}
int map_page(main_page_table *table, uint64_t phys_addr, uint64_t virt_addr, uint64_t flags)
{
    uint64_t pml4_entry = PML4_GET_INDEX(virt_addr);
    uint64_t pdpt_entry = PDPT_GET_INDEX(virt_addr);
    uint64_t pd_entry = PAGE_DIR_GET_INDEX(virt_addr);
    uint64_t pt_entry = PAGE_TABLE_GET_INDEX(virt_addr);

    main_page_table *pdpt, *pd, *pt;

    if (table[pml4_entry] & BIT_PRESENT)
    {
        pdpt = get_mem_addr<uint64_t *>(table[pml4_entry] & FRAME_ADDR);
    }
    else
    {
        pdpt = get_mem_addr<uint64_t *>(pmm_alloc_zero(1));
        table[pml4_entry] = (get_rmem_addr((uint64_t)pdpt)) | PAGE_TABLE_FLAGS;
    }

    if (pdpt[pdpt_entry] & BIT_PRESENT)
    {
        pd = get_mem_addr<uint64_t *>(pdpt[pdpt_entry] & FRAME_ADDR);
    }
    else
    {
        pd = get_mem_addr<uint64_t *>(pmm_alloc_zero(1));
        pdpt[pdpt_entry] = get_rmem_addr(pd) | PAGE_TABLE_FLAGS;
    }

    if (pd[pd_entry] & BIT_PRESENT)
    {
        pt = get_mem_addr<uint64_t *>(pd[pd_entry] & FRAME_ADDR);
    }
    else
    {
        pt = get_mem_addr<uint64_t *>(pmm_alloc_zero(1));
        pd[pd_entry] = get_rmem_addr(pt) | PAGE_TABLE_FLAGS;
    }

    pt[pt_entry] = (uint64_t)(phys_addr | flags);

    return 0;
}

uint64_t page_addr(uint64_t virt_addr)
{
    uint64_t pml4_entry = PML4_GET_INDEX(virt_addr);
    uint64_t pdpt_entry = PDPT_GET_INDEX(virt_addr);
    uint64_t pd_entry = PAGE_DIR_GET_INDEX(virt_addr);
    uint64_t pt_entry = PAGE_TABLE_GET_INDEX(virt_addr);

    main_page_table *pdpt, *pd, *pt;

    if (get_current_cpu()->page_table[pml4_entry] & BIT_PRESENT)
    {
        pdpt = get_mem_addr<uint64_t *>(get_current_cpu()->page_table[pml4_entry] & FRAME_ADDR);
    }
    else
    {
        return 0;
    }

    if (pdpt[pdpt_entry] & BIT_PRESENT)
    {
        pd = get_mem_addr<uint64_t *>(pdpt[pdpt_entry] & FRAME_ADDR);
    }
    else
    {
        return 0;
    }

    if (pd[pd_entry] & BIT_PRESENT)
    {
        pt = get_mem_addr<uint64_t *>(pd[pd_entry] & FRAME_ADDR);
    }
    else
    {
        return 0;
    }
    return pt[pt_entry] & ~(0x1000);
}

int map_page(uint64_t phys_addr, uint64_t virt_addr, uint64_t flags)
{
    uint64_t pml4_entry = PML4_GET_INDEX(virt_addr);
    uint64_t pdpt_entry = PDPT_GET_INDEX(virt_addr);
    uint64_t pd_entry = PAGE_DIR_GET_INDEX(virt_addr);
    uint64_t pt_entry = PAGE_TABLE_GET_INDEX(virt_addr);

    main_page_table *pdpt, *pd, *pt;

    if (get_current_cpu()->page_table[pml4_entry] & BIT_PRESENT)
    {
        pdpt = get_mem_addr<uint64_t *>(get_current_cpu()->page_table[pml4_entry] & FRAME_ADDR);
    }
    else
    {
        pdpt = (uint64_t *)get_mem_addr(pmm_alloc_zero(1));
        get_current_cpu()->page_table[pml4_entry] = (uint64_t)(get_rmem_addr((uint64_t)pdpt)) | PAGE_TABLE_FLAGS;
    }

    if (pdpt[pdpt_entry] & BIT_PRESENT)
    {
        pd = get_mem_addr<uint64_t *>(pdpt[pdpt_entry] & FRAME_ADDR);
    }
    else
    {
        pd = get_mem_addr<uint64_t *>(pmm_alloc_zero(1));
        pdpt[pdpt_entry] = get_rmem_addr(pd) | PAGE_TABLE_FLAGS;
    }

    if (pd[pd_entry] & BIT_PRESENT)
    {
        pt = get_mem_addr<uint64_t *>(pd[pd_entry] & FRAME_ADDR);
    }
    else
    {
        pt = get_mem_addr<uint64_t *>(pmm_alloc_zero(1));
        pd[pd_entry] = get_rmem_addr(pt) | PAGE_TABLE_FLAGS;
    }

    pt[pt_entry] = (uint64_t)(phys_addr | flags);

    return 0;
}

main_page_table *new_vmm_page_dir()
{
    main_page_table *ret_pml4 = get_mem_addr<main_page_table *>(pmm_alloc_zero(1));
    for (int i = 0; i < 512; i++)
    {
        ret_pml4[i] = 0x0;
    }

    for (int i = 255; i < 512; i++)
    {
        ret_pml4[i] = get_current_cpu()->page_table[i];
    }

    for (uint64_t i = 0; i < (TWO_MEGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;

        map_page(ret_pml4, addr, addr, BASIC_PAGE_FLAGS);
        map_page(ret_pml4, addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
        map_page(ret_pml4, addr, get_kern_addr(addr), BASIC_PAGE_FLAGS);
    }

    for (uint64_t i = TWO_MEGS / PAGE_SIZE; i < (FOUR_GIGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        map_page(ret_pml4, addr, (addr), BASIC_PAGE_FLAGS);
        map_page(ret_pml4, addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
    }

    return ret_pml4;
}

void set_vmm_page_dir(main_page_table *table)
{
    get_current_cpu()->page_table = table;
    update_paging();
}
void set_vmm_kernel_page_dir()
{
    get_current_cpu()->page_table = kernel_super_dir;
    update_paging();
}
void init_vmm(stivale_struct *bootdata)
{
    log("vmm", LOG_DEBUG) << "loading vmm";
    get_current_cpu()->page_table = get_mem_addr<uint64_t *>(pmm_alloc_zero(1));
    main_page_table *table = get_current_cpu()->page_table;
    e820_entry_t *mementry = (e820_entry_t *)bootdata->memory_map_addr;

    log("vmm", LOG_INFO) << "loading vmm 2M initial data";

    for (uint64_t i = 0; i < (TWO_MEGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        map_page(table, addr, addr, BASIC_PAGE_FLAGS);
        map_page(table, addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
        map_page(table, addr, get_kern_addr(addr), BASIC_PAGE_FLAGS);
    }

    set_paging_dir(get_rmem_addr(get_current_cpu()->page_table));

    log("vmm", LOG_INFO) << "loading vmm 4G initial data";

    for (uint64_t i = 0; i < (FOUR_GIGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        map_page(table, addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
        map_page(table, addr, (addr), BASIC_PAGE_FLAGS);
    }

    log("vmm", LOG_INFO) << "loading vmm with memory entries";

    for (uint64_t i = 0; i < bootdata->memory_map_entries; i++)
    {

        uint64_t aligned_base = mementry[i].base - (mementry[i].base % PAGE_SIZE);
        uint64_t aligned_length = ((mementry[i].length / PAGE_SIZE) + 1) * PAGE_SIZE;

        for (uint64_t j = 0; j * PAGE_SIZE < aligned_length; j++)
        {
            uint64_t addr = aligned_base + j * PAGE_SIZE;

            if (addr < FOUR_GIGS)
            {
                continue;
            }

            map_page(table, addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
        }
    }

    set_paging_dir(get_rmem_addr(get_current_cpu()->page_table));

    log("vmm", LOG_INFO) << "loading vmm done";
}
void update_paging()
{
    set_paging_dir(get_rmem_addr(get_current_cpu()->page_table));
}
