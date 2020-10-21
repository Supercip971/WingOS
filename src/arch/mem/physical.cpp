#include <arch/lock.h>
#include <arch/mem/physical.h>
#include <com.h>
#include <kernel.h>
#include <logging.h>
extern "C" uint64_t kernel_end;
extern "C" uint64_t addr_kernel_start;
lock_type pmm_lock = {0};
uint64_t bitmap_base = 0;
uint8_t *bitmap;
uint64_t available_memory;
uint64_t pmm_length = 0;
uint64_t pmm_page_entry_count = 0;
uint64_t last_free_byte = 0;
uint64_t used = 0;

static void pmm_set_bit(uint64_t page)
{
    uint8_t bit = page % 8;
    uint64_t byte = page / 8;

    bitmap[byte] |= (1 << bit);
}

static void pmm_clear_bit(uint64_t page)
{
    uint8_t bit = page % 8;
    uint64_t byte = page / 8;

    bitmap[byte] &= ~(1 << bit);
}

static uint8_t pmm_get_bit(uint64_t page)
{
    uint8_t bit = page % 8;
    uint64_t byte = page / 8;

    return bitmap[byte] & (1 << bit);
}

uint64_t pmm_find_free(uint64_t lenght)
{
    uint64_t lenght_found = 0;
    uint64_t result = 0;
    // find bitmap base
    for (uint64_t i = 0; i < pmm_length; i++)
    {
        if (!pmm_get_bit(i))
        {
            if (lenght_found == 0)
            {
                result = i;
            }
            lenght_found++;
        }
        else
        {
            result = 0;
            lenght_found = 0;
        }

        if (lenght_found == lenght)
        {
            return result;
        }
    }
    log("pmm", LOG_FATAL) << " no free protected memory found memory length " << pmm_length;
    while (true)
    {
        // do nothing
    }
    return 0x0;
}

uint64_t pmm_find_free_fast(uint64_t lenght)
{
    int lenght_found = 0;
    uint64_t result = 0;
    // find bitmap base
    for (uint64_t i = last_free_byte; i < pmm_length; i++)
    {
        if (!pmm_get_bit(i))
        {
            if (lenght_found == 0)
            {
                result = i;
            }
            lenght_found++;
        }
        else
        {
            result = 0;
            lenght_found = 0;
        }

        if (lenght_found == lenght)
        {
            last_free_byte = result + lenght_found;
            return result;
        }
    }
    log("pmm", LOG_INFO) << "free protected memory top reached [ for fast ]";
    last_free_byte = 0;
    return pmm_find_free(lenght);
    return 0x0;
}

void *pmm_alloc(uint64_t lenght)
{
    used += lenght;
    uint64_t res = pmm_find_free(lenght);

    for (uint64_t i = 0; i < lenght; i++)
    {
        pmm_set_bit(res + i);
    }

    return (void *)(res * PAGE_SIZE);
}

void *pmm_alloc_fast(uint64_t lenght)
{
    used += lenght;
    uint64_t res = pmm_find_free_fast(lenght);

    for (uint64_t i = 0; i < lenght; i++)
    {
        pmm_set_bit(res + i);
    }

    return (void *)(res * PAGE_SIZE);
}

void *pmm_alloc_zero(uint64_t lenght)
{
    void *d = pmm_alloc_fast(lenght);
    uint64_t *pages = reinterpret_cast<uint64_t *>(get_mem_addr((uint64_t)d));

    for (uint64_t i = 0; i < (lenght * PAGE_SIZE) / sizeof(uint64_t); i++)
    {
        pages[i] = 0;
    }

    return d;
}

void pmm_free(void *where, uint64_t lenght)
{
    used -= lenght;
    uint64_t where_aligned = (uint64_t)where;
    where_aligned /= PAGE_SIZE;

    for (uint64_t i = 0; i < lenght; i++)
    {
        pmm_clear_bit(where_aligned + i);
    }
}

void init_physical_memory(stivale_struct *bootdata)
{
    log("pmm", LOG_DEBUG) << "loading pmm";
    e820_entry_t *mementry = reinterpret_cast<e820_entry_t *>(bootdata->memory_map_addr);
    uint64_t total_memory_lenght = 0;

    available_memory = 0;

    bitmap_base = kernel_end + 0x1000; // if we doesn't found a free entry we try to put it into the top of the kernel (even if it is a bad idea)
    bitmap_base /= PAGE_SIZE;
    bitmap_base *= PAGE_SIZE;

    // align everything
    if (reinterpret_cast<uint64_t>(bootdata) > bitmap_base)
    {
        bitmap_base = reinterpret_cast<uint64_t>(bootdata) + sizeof(stivale_struct);
    }

    total_memory_lenght = mementry[bootdata->memory_map_entries - 1].length + mementry[bootdata->memory_map_entries - 1].base;

    log("pmm", LOG_INFO) << "finding physical memory mem map entry";
    // find a free mementry, and then put the bitmap here :O
    for (uint64_t i = 0; i < bootdata->memory_map_entries; i++)
    {
        if (mementry[i].type == 1)
        {
            if (mementry[i].length > (total_memory_lenght / PAGE_SIZE) / 8)
            {

                log("pmm", LOG_INFO) << "memory entry used " << i;
                log("pmm", LOG_INFO) << "total bitmap length" << (total_memory_lenght / PAGE_SIZE) / 8;

                bitmap_base = mementry[i].base + PAGE_SIZE;
                log("pmm", LOG_INFO) << "bitmap addr" << bitmap_base;
                pmm_length = ((total_memory_lenght / PAGE_SIZE) / 8);
                mementry[i].base += ((total_memory_lenght / PAGE_SIZE) / 8) + PAGE_SIZE + PAGE_SIZE;
                break;
            }
        }
    }

    log("pmm", LOG_DEBUG) << "loading pmm memory map";

    bitmap_base /= PAGE_SIZE;
    bitmap_base *= PAGE_SIZE;
    bitmap = reinterpret_cast<uint8_t *>(bitmap_base);

    memset(bitmap, 0xff, (total_memory_lenght / PAGE_SIZE) / 8);

    for (uint64_t i = 0; i < bootdata->memory_map_entries; i++)
    {

        for (uint64_t ij = mementry[i].base; ij < mementry[i].length + mementry[i].base; ij += PAGE_SIZE)
        {
            if (mementry[i].type != 1)
            {
                pmm_set_bit(ij / PAGE_SIZE);
            }
            else
            {
                pmm_clear_bit(ij / PAGE_SIZE);
                available_memory += 4096;
            }
        }
        pmm_page_entry_count += mementry[i].length / PAGE_SIZE;
    }

    log("pmm", LOG_INFO) << "free memory " << available_memory;
}
