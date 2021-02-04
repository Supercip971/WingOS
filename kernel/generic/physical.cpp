#include <arch.h>
#include <bitmap.h>
#include <com.h>
#include <kernel.h>
#include <logging.h>
#include <physical.h>
#include <utility.h>
bitmap pmm_bitmap;
extern "C" uint64_t kernel_end;
extern "C" uint64_t addr_kernel_start;
lock_type pmm_lock = {0};
uint64_t available_memory;
uint64_t bitmap_base;
uint64_t pmm_length = 0;
uint64_t pmm_page_entry_count = 0;
uint64_t last_free_byte = 0;
uint64_t used_memory = 0;

uint64_t get_used_memory()
{
    return used_memory;
}
uint64_t get_total_memory()
{
    return available_memory;
}

void *pmm_alloc(uint64_t lenght)
{
    flock(&pmm_lock);
    used_memory += lenght;
    if (used_memory >= available_memory)
    {
        log("pmm", LOG_WARNING) << "too much memory used" << used_memory << "/" << available_memory;
    }

    uint64_t res = pmm_bitmap.alloc(lenght);

    unlock(&pmm_lock);
    return (void *)(res * PAGE_SIZE);
}

void *pmm_alloc_zero(uint64_t lenght)
{
    void *d = pmm_alloc(lenght);
    flock(&pmm_lock);
    uint64_t *pages = (get_mem_addr<uint64_t *>((uint64_t)d));

    for (uint64_t i = 0; i < (lenght * PAGE_SIZE) / sizeof(uint64_t); i++)
    {
        pages[i] = 0;
    }
    unlock(&pmm_lock);

    return d;
}

void pmm_free(void *where, uint64_t lenght)
{
    flock(&pmm_lock);
    used_memory -= lenght;
    uint64_t where_aligned = (uint64_t)where;
    where_aligned /= PAGE_SIZE;
    if ((uint64_t)where > bitmap_base && (uint64_t)where < bitmap_base + pmm_length)
    {
        log("pmm", LOG_ERROR) << "you are freeing bitmap memory";

        return;
    }
    pmm_bitmap.set_free((uint64_t)where / PAGE_SIZE, lenght);
    unlock(&pmm_lock);
}

void init_physical_memory(stivale_struct *bootdata)
{
    log("pmm", LOG_DEBUG) << "loading pmm";
    e820_entry_t *mementry = reinterpret_cast<e820_entry_t *>(bootdata->memory_map_addr);
    uint64_t total_memory_lenght = 0;

    available_memory = 0;

    bitmap_base = ALIGN_UP(kernel_end + 0x1000, PAGE_SIZE); // if we doesn't found a free entry we try to put it into the top of the kernel (even if it is a bad idea)

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
                pmm_length = ((total_memory_lenght / PAGE_SIZE));
                mementry[i].base += ((total_memory_lenght / PAGE_SIZE) / 8) + PAGE_SIZE + PAGE_SIZE;
                break;
            }
        }
    }
    pmm_bitmap = bitmap(reinterpret_cast<uint8_t *>(bitmap_base), pmm_length);

    log("pmm", LOG_DEBUG) << "loading pmm memory map";

    for (uint64_t i = 0; i < bootdata->memory_map_entries; i++)
    {

        if (mementry[i].type != MEMMAP_USABLE)
        {
            pmm_bitmap.set_used(mementry[i].base / PAGE_SIZE, mementry[i].length / PAGE_SIZE);
        }
        else
        {
            pmm_bitmap.set_free(mementry[i].base / PAGE_SIZE, mementry[i].length / PAGE_SIZE);
            available_memory += mementry[i].length / PAGE_SIZE;
        }
        pmm_page_entry_count += mementry[i].length / PAGE_SIZE;
    }
    pmm_bitmap.reset_last_free();
    log("pmm", LOG_INFO) << "free memory " << available_memory;
}
