#include <arch.h>

#include <bitmap.h>
#include <com.h>
#include <device/acpi.h>
#include <device/local_data.h>
#include <kernel.h>
#include <physical.h>
#include <stivale_struct.h>
#include <utility.h>
#include <virtual.h>
uint64_t entry_to_address(uint64_t pml4, uint64_t pdpt, uint64_t pd, uint64_t pt)
{
    uint64_t result = 0;

    result |= pml4 << 39;
    result |= pdpt << 30;
    result |= pd << 21;
    result |= pt << 12;

    return result;
}
uint64_t get_physical_addr(uint64_t virt)
{

    uint64_t pml4_entry = PML4_GET_INDEX(virt);
    uint64_t pdpt_entry = PDPT_GET_INDEX(virt);
    uint64_t pd_entry = PAGE_DIR_GET_INDEX(virt);
    uint64_t pt_entry = PAGE_TABLE_GET_INDEX(virt);

    page_table *pdpt, *pd, *pt;
    if (get_current_cpu()->cpu_page_table[pml4_entry].is_present())
    {
        pdpt = get_mem_addr<page_table *>(get_current_cpu()->cpu_page_table[pml4_entry].get_addr());
    }
    else
    {
        return virt;
    }

    if (pdpt[pdpt_entry].is_present())
    {
        pd = get_mem_addr<page_table *>(pdpt[pdpt_entry].get_addr());
    }
    else
    {
        return virt;
    }

    if (pd[pd_entry].is_present())
    {
        pt = get_mem_addr<page_table *>(pd[pd_entry].get_addr());
    }
    else
    {
        return virt;
    }

    return pt[pt_entry].get_addr();
}
int map_page(page_table *table, uint64_t phys_addr, uint64_t virt_addr, bool is_writable, bool is_user)
{
    uint64_t pml4_entry = PML4_GET_INDEX(virt_addr);
    uint64_t pdpt_entry = PDPT_GET_INDEX(virt_addr);
    uint64_t pd_entry = PAGE_DIR_GET_INDEX(virt_addr);
    uint64_t pt_entry = PAGE_TABLE_GET_INDEX(virt_addr);

    page_table *pdpt, *pd, *pt;

    if (table[pml4_entry].is_present())
    {
        pdpt = get_mem_addr<page_table *>(table[pml4_entry].get_addr());
    }
    else
    {
        pdpt = get_mem_addr<page_table *>(pmm_alloc_zero(1));
        table[pml4_entry] = page_table::create(get_rmem_addr(pdpt), true, true);
    }

    if (pdpt[pdpt_entry].is_present())
    {
        pd = get_mem_addr<page_table *>(pdpt[pdpt_entry].get_addr());
    }
    else
    {
        pd = get_mem_addr<page_table *>(pmm_alloc_zero(1));
        pdpt[pdpt_entry] = page_table::create(get_rmem_addr(pd), true, true);
    }

    if (pd[pd_entry].is_present())
    {
        pt = get_mem_addr<page_table *>(pd[pd_entry].get_addr());
    }
    else
    {
        pt = get_mem_addr<page_table *>(pmm_alloc_zero(1));
        pd[pd_entry] = page_table::create(get_rmem_addr(pt), true, true);
    }

    pt[pt_entry] = page_table::create(phys_addr, is_writable, is_user);

    return 0;
}

uint64_t page_addr(uint64_t virt_addr)
{
    uint64_t pml4_entry = PML4_GET_INDEX(virt_addr);
    uint64_t pdpt_entry = PDPT_GET_INDEX(virt_addr);
    uint64_t pd_entry = PAGE_DIR_GET_INDEX(virt_addr);
    uint64_t pt_entry = PAGE_TABLE_GET_INDEX(virt_addr);

    page_table *pdpt, *pd, *pt;

    if (get_current_cpu()->cpu_page_table[pml4_entry].is_present())
    {
        pdpt = get_mem_addr<page_table *>(get_current_cpu()->cpu_page_table[pml4_entry].get_addr());
    }
    else
    {
        return 0;
    }

    if (pdpt[pdpt_entry].is_present())
    {
        pd = get_mem_addr<page_table *>(pdpt[pdpt_entry].get_addr());
    }
    else
    {
        return 0;
    }

    if (pd[pd_entry].is_present())
    {
        pt = get_mem_addr<page_table *>(pd[pd_entry].get_addr());
    }
    else
    {
        return 0;
    }
    return pt[pt_entry].get_addr();
}

int map_page(uint64_t phys_addr, uint64_t virt_addr, bool is_writable, bool is_user)
{
    return map_page((get_current_cpu()->cpu_page_table), phys_addr, virt_addr, is_writable, is_user);
}
page_table *new_vmm_page_dir()
{
    page_table *ret_pml4 = get_mem_addr<page_table *>(pmm_alloc_zero(1));

    for (int i = 255; i < 512; i++)
    {
        ret_pml4[i] = get_current_cpu()->cpu_page_table[i];
    }

    for (uint64_t i = 0; i < (TWO_MEGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;

        map_page(ret_pml4, addr, addr, true, false);
        map_page(ret_pml4, addr, get_mem_addr(addr), true, false);
        map_page(ret_pml4, addr, get_kern_addr(addr), true, false);
    }

    for (uint64_t i = TWO_MEGS / PAGE_SIZE; i < (FOUR_GIGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        map_page(ret_pml4, addr, (addr), true, true);
    }

    return ret_pml4;
}

void init_vmm(stivale_struct *bootdata)
{
    log("vmm", LOG_DEBUG) << "loading vmm";
    get_current_cpu()->cpu_page_table = get_mem_addr<page_table *>(pmm_alloc_zero(1));
    page_table *table = get_current_cpu()->cpu_page_table;
    e820_entry_t *mementry = (e820_entry_t *)bootdata->memory_map_addr;

    log("vmm", LOG_INFO) << "loading vmm 2M initial data";

    for (uint64_t i = 0; i < (TWO_MEGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        map_page(table, addr, addr, true, false);
        map_page(table, addr, get_mem_addr(addr), true, false);
        map_page(table, addr, get_kern_addr(addr), true, false);
    }

    set_paging_dir(get_rmem_addr(get_current_cpu()->cpu_page_table));

    log("vmm", LOG_INFO) << "loading vmm 4G initial data";

    for (uint64_t i = 0; i < (FOUR_GIGS / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        map_page(table, addr, get_mem_addr(addr), true, true);
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

            map_page(table, addr, get_mem_addr(addr), true, false);
        }
    }

    set_paging_dir(get_rmem_addr(get_current_cpu()->cpu_page_table));

    log("vmm", LOG_INFO) << "loading vmm done";
}
void update_paging()
{
    set_paging_dir(get_rmem_addr(get_current_cpu()->cpu_page_table));
}
