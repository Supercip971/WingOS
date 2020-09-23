#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/physical.h>
#include <arch/mem/virtual.h>
#include <com.h>
#include <device/acpi.h>
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
        pdpt = (uint64_t *)(get_mem_addr(table[pml4_entry] & FRAME_ADDR));
    }
    else
    {
        pdpt = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        table[pml4_entry] = (uint64_t)(get_rmem_addr((uint64_t)pdpt)) | PAGE_TABLE_FLAGS;
    }

    if (pdpt[pdpt_entry] & BIT_PRESENT)
    {
        pd = (uint64_t *)(get_mem_addr(pdpt[pdpt_entry] & FRAME_ADDR));
    }
    else
    {
        pd = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pdpt[pdpt_entry] = (uint64_t)get_rmem_addr((uint64_t)pd) | PAGE_TABLE_FLAGS;
    }

    if (pd[pd_entry] & BIT_PRESENT)
    {
        pt = (uint64_t *)(get_mem_addr(pd[pd_entry] & FRAME_ADDR));
    }
    else
    {
        pt = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pd[pd_entry] = (uint64_t)get_rmem_addr((uint64_t)pt) | PAGE_TABLE_FLAGS;
    }

    pt[pt_entry] = (uint64_t)(phys_addr | flags);

    return 0;
}
int map_page(uint64_t phys_addr, uint64_t virt_addr, uint64_t flags)
{
    uint64_t pml4_entry = PML4_GET_INDEX(virt_addr);
    uint64_t pdpt_entry = PDPT_GET_INDEX(virt_addr);
    uint64_t pd_entry = PAGE_DIR_GET_INDEX(virt_addr);
    uint64_t pt_entry = PAGE_TABLE_GET_INDEX(virt_addr);

    main_page_table *pdpt, *pd, *pt;

    if (main_pml4[pml4_entry] & BIT_PRESENT)
    {
        pdpt = (uint64_t *)(get_mem_addr(main_pml4[pml4_entry] & FRAME_ADDR));
    }
    else
    {
        pdpt = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        main_pml4[pml4_entry] = (uint64_t)(get_rmem_addr((uint64_t)pdpt)) | PAGE_TABLE_FLAGS;
    }

    if (pdpt[pdpt_entry] & BIT_PRESENT)
    {
        pd = (uint64_t *)(get_mem_addr(pdpt[pdpt_entry] & FRAME_ADDR));
    }
    else
    {
        pd = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pdpt[pdpt_entry] = (uint64_t)get_rmem_addr((uint64_t)pd) | PAGE_TABLE_FLAGS;
    }

    if (pd[pd_entry] & BIT_PRESENT)
    {
        pt = (uint64_t *)(get_mem_addr(pd[pd_entry] & FRAME_ADDR));
    }
    else
    {
        pt = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pd[pd_entry] = (uint64_t)get_rmem_addr((uint64_t)pt) | PAGE_TABLE_FLAGS;
    }

    pt[pt_entry] = (uint64_t)(phys_addr | flags);

    return 0;
}
main_page_table *fork_vmm_page_dir()
{

    main_page_table *ret_pml4 = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));

    uint64_t *pdpt;
    uint64_t *pd;
    uint64_t *pt;
    for (int i = 0; i < 512; i++)
    { // pml4
        if (main_pml4[i] & 1)
        {
            pdpt = (uint64_t *)(get_mem_addr(main_pml4[i] & FRAME_ADDR));
            for (int j = 0; j < 512; j++)
            { // pdpt
                if (pdpt[j] & 1)
                {
                    pd = (uint64_t *)(get_mem_addr(pdpt[j] & FRAME_ADDR));
                    for (int k = 0; k < 512; k++)
                    { // pd
                        if (pd[k] & 1)
                        {
                            pt = (uint64_t *)(get_mem_addr(pd[k] & FRAME_ADDR));
                            for (int l = 0; l < 512; l++)
                            { // pt
                                /* see that mountain */
                                if (pt[l] & 1)
                                {
                                    /* oh there is a cave ! */ //  map_page(ret_pml4, , entry_to_address(i,j,k,l), 0x03, pt[l] & 0xfff);
                                    uint64_t end_virtual_addres = entry_to_address(i, j, k, l);
                                    uint64_t end_physical_addres = pt[l] & ~(0xfff);
                                    //         void* new_physical = pmm_alloc(1);
                                    map_page(ret_pml4, end_physical_addres, end_virtual_addres, pt[l] & (0xfff));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ret_pml4;
}
void set_vmm_page_dir(main_page_table *table)
{
    main_pml4 = table;
    update_paging();
}
void set_vmm_kernel_page_dir()
{
    main_pml4 = kernel_super_dir;
    update_paging();
}
void init_vmm(stivale_struct *bootdata)
{
    log("vmm", LOG_DEBUG) << "loading vmm";
    main_pml4 = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));

    e820_entry_t *mementry = (e820_entry_t *)bootdata->memory_map_addr;

    log("vmm", LOG_INFO) << "loading vmm 2M initial data";
    for (uint64_t i = 0; i < (TWO_MEGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        map_page(addr, addr, BASIC_PAGE_FLAGS);
        map_page(addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
        map_page(addr, get_kern_addr(addr), BASIC_PAGE_FLAGS | (1 << 8));
    }

    set_paging_dir(get_rmem_addr((uint64_t)main_pml4));
    log("vmm", LOG_INFO) << "loading vmm 4G initial data";
    for (uint64_t i = 0; i < (FOUR_GIGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;

        map_page(addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
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
                continue;

            map_page(addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
        }
    }

    set_paging_dir(get_rmem_addr((uint64_t)main_pml4));
   // main_pml4 = fork_vmm_page_dir(); for testing
   // set_paging_dir(get_rmem_addr((uint64_t)main_pml4));

    log("vmm", LOG_INFO) << "loading vmm done";
}
