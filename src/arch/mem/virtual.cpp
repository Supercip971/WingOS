#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/virtual.h>
#include <com.h>
#include <kernel.h>
#include <stivale.h>
#include <utility.h>

#define MEMORY_BASE 0x1000000
#define BITMAP_BASE (MEMORY_BASE / PAGE_SIZE)

uint64_t max_mem = 0;
extern "C" uint64_t kernel_end;
uint64_t frame_end = 0;
uint32_t *frames = 0;
uint64_t frames_counter = 32;
uint64_t heap_start = 0;
uint64_t heap_end = 0;
uint64_t frame_cursor_pos = 0;

uint8_t temp_alloc_ptr[0x2000000]; // 33 megs of data so
uint32_t temp_alloc_current_cursor = 0;
bool paging_is_initialized;

uint64_t align_up(uint64_t num, uint64_t multiple)
{
    return ((num + multiple - 1) / multiple) * multiple;
}

void frame_set(uint64_t address)
{
    address -= BITMAP_BASE;
    uint64_t frame = address / PAGE_SIZE;

    uint64_t idx = INDEX_FROM_BIT(frame);

    uint64_t offset = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << offset);
}

int test_frame_bit(uint64_t offset)
{
    int ret = -1;
    asm volatile("bt [%1], %2;"
                 : "=@ccc"(ret)
                 : "r"(frames), "r"(offset)
                 : "memory");
    return ret;
}
int set_frame_bit(uint64_t offset)
{
    int ret = -1;
    asm volatile("bts [%1], %2;"
                 : "=@ccc"(ret)
                 : "r"(frames), "r"(offset)
                 : "memory");
    return ret;
}
int reset_frame_bit(uint64_t offset)
{
    int d = 0;
    asm volatile("btr [%1], %2;"
                 : "=@ccc"(d)
                 : "r"(frames), "r"(offset)
                 : "memory");
    return 0;
}

__attribute__((always_inline)) static inline void
set_frame_region(uint64_t i, uint64_t count)
{
    i -= BITMAP_BASE;
    size_t f = i + count;
    for (size_t j = i; j < f; j++)
        set_frame_bit(j);
}
__attribute__((always_inline)) static inline void
unset_frame_region(uint64_t i, uint64_t count)
{
    i -= BITMAP_BASE;
    size_t f = i + count;
    for (size_t j = i; j < f; j++)
        reset_frame_bit(j);
}

void pmm_free(void *ptr, uint64_t pg_count)
{
    uint64_t start = (uint64_t)ptr / PAGE_SIZE;

    unset_frame_region(start, pg_count);
}
uint64_t get_mem_addr(uint64_t addr) { return addr + 0xffff800000000000; }
uint64_t get_rmem_addr(uint64_t addr) { return addr - 0xffff800000000000; }
uint64_t get_kern_addr(uint64_t addr) { return addr + 0xffffffff80000000; }
uint64_t get_rkern_addr(uint64_t addr) { return addr - 0xffffffff80000000; }
#define PAGE_FRAME 0xFFFFFFFFFF000

void frame_clear(uint64_t address)
{
    uint64_t frame = address / PAGE_SIZE;

    uint64_t idx = INDEX_FROM_BIT(frame);

    uint64_t offset = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << offset);
}

uint64_t mmu_frame_test(uint64_t frame_addr)
{
    uint64_t frame = frame_addr / PAGE_SIZE;
    uint64_t idx = INDEX_FROM_BIT(frame);
    uint64_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}
uint32_t very_initial_frame_table[] = {0xffffff7f};
uint32_t *temp_frame_table;
void init_frame(uint64_t lenght, stivale_struct *sti_struct)
{
    com_write_str("loading initial frame table");
    frames_counter = 32;
    frames = very_initial_frame_table;
    temp_frame_table = (uint32_t *)alloc_multiple_frame_zero(1);
    temp_frame_table = (uint32_t *)(get_mem_addr((uint64_t)temp_frame_table));
    for (uint64_t i = 0; i < (PAGE_SIZE) / sizeof(uint32_t); i++)
    {
        temp_frame_table[i] = 0xffffffff;
    }
    frames = temp_frame_table;

    frames_counter = ((PAGE_SIZE / sizeof(uint32_t)) * 32);

    e820_entry_t *mementry = (e820_entry_t *)sti_struct->memory_map_addr;
    for (size_t i = 0; i < sti_struct->memory_map_entries; i++)
    {
        e820_entry_t *entry = &mementry[i];

        uint64_t aligned_base;
        if (entry->base % PAGE_SIZE)
        {
            aligned_base = entry->base + (PAGE_SIZE - (entry->base % PAGE_SIZE));
        }
        else
        {
            aligned_base = entry->base;
        }

        size_t aligned_length = (entry->length / PAGE_SIZE) * PAGE_SIZE;
        if ((entry->base % PAGE_SIZE) && aligned_length)
        {
            aligned_length -= PAGE_SIZE;
        }

        for (size_t j = 0; j * PAGE_SIZE < aligned_length; j++)
        {
            uint64_t addr = aligned_base + j * PAGE_SIZE;
            size_t page = addr / PAGE_SIZE;
            if (addr < (MEMORY_BASE + PAGE_SIZE /* bitmap */))
                continue;
            if (addr >= (MEMORY_BASE + frames_counter * PAGE_SIZE))
            {
                /* Reallocate bitmap */
                size_t cur_bitmap_size_in_pages =
                    ((frames_counter / 32) * sizeof(uint32_t)) / PAGE_SIZE;
                size_t new_bitmap_size_in_pages = cur_bitmap_size_in_pages + 1;

                temp_frame_table =
                    (uint32_t *)alloc_multiple_frame_zero(new_bitmap_size_in_pages);

                temp_frame_table =
                    (uint32_t *)(get_mem_addr((uint64_t)temp_frame_table));
                /* Copy over previous bitmap */

                for (uint64_t i = 0;
                     i < (cur_bitmap_size_in_pages * PAGE_SIZE) / sizeof(uint32_t);
                     i++)
                {
                    temp_frame_table[i] = frames[i];
                }
                /* Fill in the rest */
                for (uint64_t i =
                         (cur_bitmap_size_in_pages * PAGE_SIZE) / sizeof(uint32_t);
                     i < (new_bitmap_size_in_pages * PAGE_SIZE) / sizeof(uint32_t);
                     i++)
                {
                    temp_frame_table[i] = 0xffffffff;
                }
                frames_counter += ((PAGE_SIZE / sizeof(uint32_t)) * 32) * 1;
                uint32_t *old_bitmap = (uint32_t *)(get_rmem_addr((uint64_t)frames));
                frames = temp_frame_table;
                pmm_free((old_bitmap), cur_bitmap_size_in_pages);
            }

            if (entry->type == MEMMAP_USABLE)
            {
                unset_frame_region(page, 1);
            }
        }
    }
    com_write_str("loading frame : OK");
}

uint64_t frame_find_first()
{
    uint64_t i, j;
    for (i = 0; i < frames_counter; i++)
    {
        if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
        {
            // at least one bit is free here.
            for (j = 0; j < 32; j++)
            {
                uint32_t toTest = 0x1 << j;
                if (!(frames[i] & toTest))
                {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
    return 0;
}

void free_frame(uint64_t ptr) { frame_clear(ptr); }

void *alloc_frame()
{
    uint64_t frame = frame_find_first();
    if (frame == 0)
    {
        com_write_str("error not enought frame :^(");
        return 0x0; // never reached
    }
    frame_set(frame * PAGE_SIZE);
    return (void *)(frame * PAGE_SIZE);
}
void *alloc_multiple_frame(uint64_t count, bool use_fast)
{
    if (use_fast)
    {
        if (frame_cursor_pos == 0)
        {
            frame_cursor_pos = BITMAP_BASE;
        }
        size_t pg_cnt = count;
        uint64_t i = 0;
        for (i = 0; i < frames_counter;)
        {
            if (frame_cursor_pos == BITMAP_BASE + frames_counter)
            {
                frame_cursor_pos = BITMAP_BASE;
                pg_cnt = count;
            }

            if (!test_frame_bit((frame_cursor_pos++) - BITMAP_BASE))
            {
                if (!--pg_cnt)
                {
                    set_frame_region(frame_cursor_pos - count, count);
                    return (void *)((frame_cursor_pos - count) * PAGE_SIZE);
                }
            }
            else
            {
                pg_cnt = count;
            }
        }

        com_write_str("error kernel doesn't have that much memory (fast)");
    }
    size_t pg_cnt = count;
    uint64_t i = BITMAP_BASE;
    for (i = BITMAP_BASE; i < BITMAP_BASE + frames_counter;)
    {
        if (!test_frame_bit((i++) - BITMAP_BASE))
        {
            if (!--pg_cnt)
            {
                set_frame_region(i - count, count);
                return (void *)((i - count) * PAGE_SIZE);
            }
        }
        else
        {
            pg_cnt = count;
        }
    }

    com_write_str("error kernel doesn't have that much memory (no frame)");
    return 0x0;
}
void *alloc_multiple_frame_zero(uint64_t count, bool use_fast)
{
    void *d = alloc_multiple_frame(count, use_fast);

    uint64_t *pages = ((uint64_t *)(get_mem_addr((uint64_t)d)));

    for (uint64_t i = 0; i < (count * PAGE_SIZE) / sizeof(uint64_t); i++)
    {
        pages[i] = 0;
    }
    return d;
}

#define PAGE_FRAME 0xFFFFFFFFFF000

inline void SetPageFrame(uint64_t *page, uint64_t addr)
{
    *page = (*page & ~PAGE_FRAME) | (addr & PAGE_FRAME);
}

inline void SetPageFlags(uint64_t *page, uint64_t flags) { *page |= flags; }

void init_paging(stivale_struct *sti_struct)
{
    com_write_str("loading paging 1");
    pl4_table =
        (uint64_t *)(get_mem_addr((uint64_t)alloc_multiple_frame_zero(1)));
    for (uint64_t i = 0; i < (0x2000000 / PAGE_SIZE); i++)
    {
        uint64_t addr = i * PAGE_SIZE;
        virt_map(addr, addr, 0x03);
        virt_map(addr, get_mem_addr(addr), 0x03);
        virt_map(addr, get_kern_addr(addr), 0x03 | (1 << 8));
    }

    com_write_str("loading paging 2");
    set_paging_dir(get_rmem_addr((uint64_t)pl4_table));

    uint64_t framebuffer_lenght = sti_struct->framebuffer_width *
                                  sti_struct->framebuffer_height *
                                  sti_struct->framebuffer_bpp;
    for (uint64_t i = (sti_struct->framebuffer_addr) / PAGE_SIZE;
         i <
         ((framebuffer_lenght + sti_struct->framebuffer_addr) / PAGE_SIZE) + 1;
         i++)
    {
        uint64_t addr = i * PAGE_SIZE;

        virt_map(addr, get_mem_addr(addr), 0x03);
    }
    com_write_str("loading paging 3");
    e820_entry_t *mementry = (e820_entry_t *)sti_struct->memory_map_addr;
    for (uint64_t i = 0; i < sti_struct->memory_map_entries; i++)
    {
        e820_entry_t *entry = &mementry[i];
        uint64_t base_aligned = entry->base - (entry->base % 4096);
        uint64_t lenght_aligned = align_up(entry->length, 4096);

        if (entry->base % PAGE_SIZE)
            lenght_aligned += PAGE_SIZE;

        for (size_t j = 0; j * PAGE_SIZE < lenght_aligned; j++)
        {
            size_t addr = base_aligned + j * PAGE_SIZE;

            /* Skip over first 4 GiB */
            if (addr < 0xffffffff)
                continue;

            virt_map(addr, get_mem_addr(addr), 0x03);
        }
    }
    set_paging_dir((uint64_t)pl4_table);
    com_write_str("loading paging done");
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
            continue;
            break;
        }
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

    com_write_str("loading frame");
    init_frame(max_mem, sti_struct);

    com_write_str("loading paging");
    init_paging(sti_struct);
}

void virt_map(uint64_t vaddress, uint64_t paddress, uint64_t flags)
{
    uint64_t _pml4e_offset = PML4_GET_INDEX(vaddress);
    uint64_t _pdpt_offset = PDPT_GET_INDEX(vaddress);
    uint64_t _pd_offset = PAGE_DIR_GET_INDEX(vaddress);
    uint64_t _pt_offset = PAGE_TABLE_GET_INDEX(vaddress);

    uint64_t *pdpt = 0x0;
    if (pl4_table[_pml4e_offset] & 0x1)
    {
        pdpt = (uint64_t *)get_mem_addr(
            (pl4_table[_pml4e_offset] & 0xfffffffffffff000));
    }
    else
    {
        pdpt =
            (uint64_t *)get_mem_addr((uint64_t)alloc_multiple_frame_zero(1, true));
        pl4_table[_pml4e_offset] =
            (uint64_t)(get_rmem_addr((uint64_t)pdpt) | 0b111);
    }

    uint64_t *pd = 0x0;
    if (pdpt[_pdpt_offset] & 0x1)
    {
        pd = (uint64_t *)get_mem_addr((pdpt[_pdpt_offset] & 0xfffffffffffff000));
    }
    else
    {
        pd = (uint64_t *)get_mem_addr((uint64_t)alloc_multiple_frame_zero(1, true));
        pdpt[_pdpt_offset] = (uint64_t)(get_rmem_addr((uint64_t)pd) | 0b111);
    }

    uint64_t *pt = 0x0;
    if (pdpt[_pd_offset] & 0x1)
    {
        pt = (uint64_t *)get_mem_addr((pd[_pd_offset] & 0xfffffffffffff000));
    }
    else
    {
        pt = (uint64_t *)get_mem_addr((uint64_t)alloc_multiple_frame_zero(1, true));
        pd[_pd_offset] = (uint64_t)(get_rmem_addr((uint64_t)pt) | 0b111);
    }
    pt[_pt_offset] = (uint64_t)(paddress | flags);
}
