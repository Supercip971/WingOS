#include <arch.h>
#include <com.h>
#include <kernel.h>
#include <logging.h>
#include <physical.h>
#include <utility.h>
#include <utils/bitmap.h>
#include <utils/lock.h>
bitmap pmm_bitmap;
extern "C" uint64_t kernel_end;
extern "C" uint64_t addr_kernel_start;
utils::lock_type pmm_lock;
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
    pmm_lock.lock();
    used_memory += lenght;
    if (used_memory >= available_memory)
    {
        log("pmm", LOG_WARNING, "too much memory used: {}/{}", used_memory, available_memory);
    }

    uint64_t res = pmm_bitmap.alloc(lenght);

    pmm_lock.unlock();
    return (void *)(res * PAGE_SIZE);
}

void *pmm_alloc_zero(uint64_t lenght)
{
    void *d = pmm_alloc(lenght);
    pmm_lock.lock();
    uint64_t *pages = (get_mem_addr<uint64_t *>((uint64_t)d));

    for (uint64_t i = 0; i < (lenght * PAGE_SIZE) / sizeof(uint64_t); i++)
    {
        pages[i] = 0;
    }
    pmm_lock.unlock();

    return d;
}

void pmm_free(void *where, uint64_t lenght)
{
    pmm_lock.lock();
    used_memory -= lenght;
    uint64_t where_aligned = (uint64_t)where;
    where_aligned /= PAGE_SIZE;
    if ((uint64_t)where > bitmap_base && (uint64_t)where < bitmap_base + pmm_length)
    {
        log("pmm", LOG_ERROR, "you are trying to free bitmap memory");

        return;
    }
    pmm_bitmap.set_free((uint64_t)where / PAGE_SIZE, lenght);
    pmm_lock.unlock();
}

size_t get_available_memory(e820_entry_t *mem_entry, size_t length)
{
    return mem_entry[length - 1].length + mem_entry[length - 1].base;
}

void init_bitmap_base(e820_entry_t *mem_entry, size_t entry_count, size_t targetted_length)
{
    for (uint64_t i = 0; i < entry_count; i++)
    {
        if (mem_entry[i].type == MEMMAP_USABLE)
        {
            if (mem_entry[i].length > (targetted_length) / 8)
            {

                log("pmm", LOG_INFO, "memory entry used: {}", i);
                log("pmm", LOG_INFO, "total bitmap length: {}", (targetted_length) / 8);
                bitmap_base = mem_entry[i].base + PAGE_SIZE;

                log("pmm", LOG_INFO, "bitmap addr: {}", bitmap_base);

                pmm_length = (targetted_length);
                mem_entry[i].base += ((targetted_length) / 8) + PAGE_SIZE + PAGE_SIZE;
                return;
            }
        }
    }
}

void init_bitmap_memory_map(e820_entry_t *mem_entry, size_t entry_count, bitmap &target)
{

    for (uint64_t i = 0; i < entry_count; i++)
    {
        log("pmm", LOG_INFO, "entry: {} -- from: {} to: {} -- status: {}", i, mem_entry[i].base, mem_entry[i].base + mem_entry[i].length, mem_entry[i].type);

        if (mem_entry[i].type == MEMMAP_USABLE)
        {
            target.set_free(mem_entry[i].base / PAGE_SIZE, mem_entry[i].length / PAGE_SIZE);
            available_memory += mem_entry[i].length / PAGE_SIZE;
        }
        pmm_page_entry_count += mem_entry[i].length / PAGE_SIZE;
    }
}

void init_physical_memory(stivale_struct *bootdata)
{
    available_memory = 0;

    log("pmm", LOG_DEBUG, "loading pmm");

    e820_entry_t *mementry = reinterpret_cast<e820_entry_t *>(bootdata->memory_map_addr);
    uint64_t total_memory_lenght = get_available_memory(mementry, bootdata->memory_map_entries);

    bitmap_base = reinterpret_cast<uintptr_t>(bootdata) + sizeof(stivale_struct);

    log("pmm", LOG_INFO, "finding physical memory mem map entry");
    init_bitmap_base(mementry, bootdata->memory_map_entries, total_memory_lenght / PAGE_SIZE);

    pmm_bitmap = bitmap(reinterpret_cast<uint8_t *>(bitmap_base), pmm_length);

    log("pmm", LOG_DEBUG, "loading pmm memory map");

    init_bitmap_memory_map(mementry, bootdata->memory_map_entries, pmm_bitmap);
    pmm_bitmap.reset_last_free();
    log("pmm", LOG_INFO, "free memory: {}", available_memory);
}
