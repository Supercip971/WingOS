#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/virtual.h>
#include <com.h>
#include <device/acpi.h>
#include <kernel.h>
#include <stivale_struct.h>

#include <utility.h>
extern "C" uint64_t kernel_end;
uint64_t max_mem = 0;
uint64_t frame_end = 0;
uint32_t *frames = 0;
uint64_t frames_counter = 32;
uint64_t heap_start = 0;
uint64_t heap_end = 0;
uint64_t frame_cursor_pos = 0;
uint64_t previous_cr3 = 0x0;
uint32_t temp_alloc_current_cursor = 0;
bool paging_is_initialized;
uint64_t prev_cr3()
{
    return previous_cr3;
}
uint64_t align_up(uint64_t num, uint64_t multiple)
{
    return ((num + multiple - 1) / multiple) * multiple;
}

void init_paging(stivale_struct *sti_struct)
{
    printf("loading paging \n");

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

    uint64_t frame_buffer_size = sti_struct->framebuffer_width * sti_struct->framebuffer_height * sizeof(uint32_t) + PAGE_SIZE;
    for (uint64_t i = sti_struct->framebuffer_addr; i < sti_struct->framebuffer_addr + frame_buffer_size; i += PAGE_SIZE)
    {
        virt_map(i, i, BASIC_PAGE_FLAGS);
    }

    virt_map((uint64_t)(sti_struct), (uint64_t)(sti_struct), BASIC_PAGE_FLAGS);

    e820_entry_t *mementry = (e820_entry_t *)sti_struct->memory_map_addr;

    printf("mapping address space \n");
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

    virt_map(get_rmem_addr((uint64_t)pl4_table), get_rmem_addr((uint64_t)pl4_table), BASIC_PAGE_FLAGS);
    set_paging_dir(get_rmem_addr((uint64_t)pl4_table));
    printf("loading paging done \n");
}

void update_paging()
{
    set_paging_dir(get_rmem_addr((uint64_t)pl4_table));
}

void init_virtual_memory(stivale_struct *sti_struct)
{
    uint64_t usable_memory = 0;
    e820_entry_t *mementry = (e820_entry_t *)sti_struct->memory_map_addr;
    for (int i = 0; i < sti_struct->memory_map_entries; i++)
    {
        printf(" ============== \n");

        switch (mementry[i].type)
        {
        case MEMMAP_USABLE:
            usable_memory += mementry[i].length;
            printf("memory usable \n");
            break;
        case MEMMAP_KERNEL_AND_MODULES:
            printf("kernel \n");
            break;
        default:
            break;
        }
        printf("memory type %x \n", mementry[i].type);
        max_mem += mementry[i].length;
        printf("memory start : %x \n", mementry[i].base);
        printf("memory end : %x \n", mementry[i].length + mementry[i].base);
        printf("memory lenght : %x \n", mementry[i].length);
    }
    printf("kernel total memory (in Mb) = %x \n", max_mem / 0xFFFFF);
    printf("kernel usable memory (in Mb) = %x \n", usable_memory / 0xFFFFF);
    printf("loading physical \n");
    init_physical_memory(sti_struct);

    printf("loading virtual \n");
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
