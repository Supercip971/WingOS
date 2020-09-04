#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/virtual.h>
#include <com.h>
#include <device/acpi.h>
#include <kernel.h>
#include <stivale_struct.h>

#include <utility.h>
#define MEMORY_BASE 0x1000000
#define BITMAP_BASE (MEMORY_BASE / PAGE_SIZE)
#define FULL_FRAME_USED 0xffffffff
#define TWO_MEGS 0x2000000
#define FOUR_GIGS 0x100000000
#define BASIC_PAGE_FLAGS 0b111
#define BIT_PRESENT 0x1
#define FRAME_ADDR 0xfffffffffffff000
uint64_t max_mem = 0;
extern "C" uint64_t kernel_end;
uint64_t frame_end = 0;
uint32_t *frames = 0;
uint64_t frames_counter = 32;
uint64_t heap_start = 0;
uint64_t heap_end = 0;
uint64_t frame_cursor_pos = 0;

uint32_t temp_alloc_current_cursor = 0;
bool paging_is_initialized;

uint64_t align_up(uint64_t num, uint64_t multiple)
{
    return ((num + multiple - 1) / multiple) * multiple;
}

void init_paging(stivale_struct *sti_struct)
{
    com_write_str("loading paging 1");
    pl4_table =
        (uint64_t *)(get_mem_addr((uint64_t)pmm_alloc_zero(1)));
    for (uint64_t i = 0; i < (0x2000000 / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        virt_map(addr, addr, BASIC_PAGE_FLAGS);
        virt_map(addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
        virt_map(addr, get_kern_addr(addr), BASIC_PAGE_FLAGS | (1 << 8));
    }

    set_paging_dir(get_rmem_addr((uint64_t)pl4_table));

    for (uint64_t i = 0x0; i < (FOUR_GIGS); i += TWO_MEGS)
    {

        Huge_virt_map(i, (i), BASIC_PAGE_FLAGS);
    }
    uint64_t frame_buffer_size = sti_struct->framebuffer_width * sti_struct->framebuffer_height * sizeof(uint32_t) + PAGE_SIZE;
    for (uint64_t i = sti_struct->framebuffer_addr; i < sti_struct->framebuffer_addr + frame_buffer_size; i += PAGE_SIZE)
    {
        virt_map(i, i, BASIC_PAGE_FLAGS);
    }
    virt_map((uint64_t)(sti_struct), (uint64_t)(sti_struct), BASIC_PAGE_FLAGS);
    com_write_str("loading paging 2");
    e820_entry_t *mementry = (e820_entry_t *)sti_struct->memory_map_addr;

    com_write_str("loading paging 3");
    for (uint64_t i = 0; i < sti_struct->memory_map_entries; i++)
    {
        e820_entry_t *entry = &mementry[i];
        uint64_t base_aligned = entry->base - (entry->base % PAGE_SIZE);
        uint64_t lenght_aligned = align_up(entry->length, PAGE_SIZE);

        if (entry->base % PAGE_SIZE)
            lenght_aligned += PAGE_SIZE;

        for (size_t j = 0; j * PAGE_SIZE < lenght_aligned; j++)
        {
            size_t addr = base_aligned + j * PAGE_SIZE;

            /* Skip over first 4 GiB */
            if (addr < FOUR_GIGS)
            {
                continue;
            }
            virt_map(addr, get_mem_addr(addr), BASIC_PAGE_FLAGS);
        }
    }
    virt_map((uint64_t)pl4_table, (uint64_t)pl4_table, BASIC_PAGE_FLAGS);
    set_paging_dir(get_rmem_addr((uint64_t)pl4_table));
    com_write_str("loading paging done");
}

void update_paging()
{

    set_paging_dir(get_rmem_addr((uint64_t)pl4_table));
}

void init_virtual_memory(stivale_struct *sti_struct)
{

    e820_entry_t *mementry = (e820_entry_t *)sti_struct->memory_map_addr;
    char buffer[64];
    memzero(buffer, sizeof(buffer));
    for (int i = 0; i < sti_struct->memory_map_entries; i++)
    {
        com_write_str(" ============== ");

        switch (mementry[i].type)
        {
        case MEMMAP_USABLE:
            com_write_str("memory usable");
            break;
        case MEMMAP_KERNEL_AND_MODULES:
            com_write_str("kernel");
            break;
        default:
            break;
        }
        com_write_reg("memory type ", mementry[i].type);
        max_mem += mementry[i].length;
        kitoaT<uint64_t>(buffer, 'x', mementry[i].base);
        com_write_str(" memory start : ");
        com_write_str(buffer);
        memzero(buffer, sizeof(buffer));
        kitoaT<uint64_t>(buffer, 'x', mementry[i].length + mementry[i].base);
        com_write_str(" memory end : ");
        com_write_str(buffer);
        memzero(buffer, sizeof(buffer));
        kitoaT<uint64_t>(buffer, 'x', mementry[i].length);
        com_write_str(" memory lenght : ");
        com_write_str(buffer);
        memzero(buffer, sizeof(buffer));
        kitoaT<uint32_t>(buffer, 'x', mementry[i].type);
        com_write_str(" memory type : ");
        com_write_str(buffer);
        memzero(buffer, sizeof(buffer));
    }
    memzero(buffer, sizeof(buffer));
    kitoaT<uint64_t>(buffer, 'd', max_mem / 0xFFFFF);
    com_write_str(" kernel memory (in Mb): ");
    com_write_str(buffer);

    com_write_str("loading physical");
    init_physical_memory(sti_struct);

    com_write_str("loading virtual");
    init_paging(sti_struct);
}

void virt_map(uint64_t vaddress, uint64_t paddress, uint64_t flags)
{
    uint64_t _pml4e_offset = PML4_GET_INDEX(vaddress);
    uint64_t _pdpt_offset = PDPT_GET_INDEX(vaddress);
    uint64_t _pd_offset = PAGE_DIR_GET_INDEX(vaddress);
    uint64_t _pt_offset = PAGE_TABLE_GET_INDEX(vaddress);

    uint64_t *pdpt = 0x0;
    if (pl4_table[_pml4e_offset] & BIT_PRESENT)
    {
        pdpt = (uint64_t *)(get_mem_addr(
                                (pl4_table[_pml4e_offset])) &
                            FRAME_ADDR);
    }
    else
    {
        pdpt =
            (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pl4_table[_pml4e_offset] =
            (uint64_t)(get_rmem_addr((uint64_t)pdpt) | BASIC_PAGE_FLAGS);
    }

    uint64_t *pd = 0x0;
    if (pdpt[_pdpt_offset] & BIT_PRESENT)
    {
        //  pd = (uint64_t *)get_mem_addr((pdpt[_pdpt_offset] & FRAME_ADDR));
        pd = (uint64_t *)(get_mem_addr(
                              (pdpt[_pdpt_offset])) &
                          FRAME_ADDR);
    }
    else
    {
        pd = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pdpt[_pdpt_offset] = (uint64_t)(get_rmem_addr((uint64_t)pd) | BASIC_PAGE_FLAGS);
    }

    uint64_t *pt = 0x0;
    if (pdpt[_pd_offset] & BIT_PRESENT)
    {
        pt = (uint64_t *)(get_mem_addr(
                              (pd[_pd_offset])) &
                          FRAME_ADDR);
        //     pt = (uint64_t *)get_mem_addr((pd[_pd_offset] & FRAME_ADDR));
    }
    else
    {
        pt = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pd[_pd_offset] = (uint64_t)(get_rmem_addr((uint64_t)pt) | BASIC_PAGE_FLAGS);
    }
    pt[_pt_offset] = (uint64_t)(paddress | flags);
}
void Huge_virt_map(uint64_t paddress, uint64_t vaddress, uint64_t flags)
{
    uint64_t _pml4e_offset = PML4_GET_INDEX(vaddress);
    uint64_t _pdpt_offset = PDPT_GET_INDEX(vaddress);
    uint64_t _pd_offset = PAGE_DIR_GET_INDEX(vaddress);

    uint64_t *pdpt = 0x0;
    if (pl4_table[_pml4e_offset] & BIT_PRESENT)
    {
        pdpt = (uint64_t *)get_mem_addr(
            (pl4_table[_pml4e_offset] & FRAME_ADDR));
    }
    else
    {
        pdpt =
            (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pl4_table[_pml4e_offset] =
            (uint64_t)(get_rmem_addr((uint64_t)pdpt) | BASIC_PAGE_FLAGS);
    }

    uint64_t *pd = 0x0;
    if (pdpt[_pdpt_offset] & BIT_PRESENT)
    {
        pd = (uint64_t *)get_mem_addr((pdpt[_pdpt_offset] & FRAME_ADDR));
    }
    else
    {
        pd = (uint64_t *)get_mem_addr((uint64_t)pmm_alloc_zero(1));
        pdpt[_pdpt_offset] = (uint64_t)(get_rmem_addr((uint64_t)pd) | BASIC_PAGE_FLAGS);
    }
    pdpt[_pdpt_offset] = (uint64_t)(paddress | flags | (1ull << 7));
}
uint64_t get_physical_address(uint64_t virtual_address)
{
    uint64_t _pml4e_offset = PML4_GET_INDEX(virtual_address);
    uint64_t _pdpt_offset = PDPT_GET_INDEX(virtual_address);
    uint64_t _pd_offset = PAGE_DIR_GET_INDEX(virtual_address);
    uint64_t _pt_offset = PAGE_TABLE_GET_INDEX(virtual_address);
    uint64_t *pdpt;
    if (pl4_table[_pml4e_offset] & BIT_PRESENT)
    {
        pdpt = (uint64_t *)get_mem_addr(
            (pl4_table[_pml4e_offset] & FRAME_ADDR));
    }
    else
    {
        return NULL;
    }

    uint64_t *pd;
    if (pdpt[_pdpt_offset] & BIT_PRESENT)
    {
        pd = (uint64_t *)get_mem_addr((pdpt[_pdpt_offset] & FRAME_ADDR));
    }
    else
    {
        return NULL;
    }

    uint64_t *pt;
    if (pdpt[_pd_offset] & BIT_PRESENT)
    {
        pt = (uint64_t *)get_mem_addr((pd[_pd_offset] & FRAME_ADDR));
    }
    else
    {
        return NULL;
    }
    return pt[_pt_offset] & ~(0xfff | 1ull << 63);
}
