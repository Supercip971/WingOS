#include <arch.h>
#include <device/debug/com.h>
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
uint64_t bitmap_base = 0;
uint64_t pmm_length = 0;
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

void *pmm_alloc(uint64_t length)
{
    pmm_lock.lock();
    used_memory += length;
    if (used_memory >= available_memory)
    {
        log("pmm", LOG_WARNING, "too much memory used: {}/{}", used_memory, available_memory);
    }

    uint64_t res = pmm_bitmap.alloc(length);

    pmm_lock.unlock();
    return (void *)(res * PAGE_SIZE);
}

void *pmm_alloc_zero(uint64_t length)
{
    void *d = pmm_alloc(length);

    memset((void *)get_mem_addr(d), 0, PAGE_SIZE * length);

    return d;
}

void pmm_free(void *where, uint64_t length)
{
    pmm_lock.lock();
    used_memory -= length;
    uint64_t where_aligned = (uint64_t)where;
    where_aligned /= PAGE_SIZE;
    if ((uint64_t)where > bitmap_base && (uint64_t)where < bitmap_base + pmm_length)
    {
        log("pmm", LOG_ERROR, "you are trying to free bitmap memory");

        return;
    }
    pmm_bitmap.set_free((uint64_t)where / PAGE_SIZE, length);
    pmm_lock.unlock();
}

size_t get_available_memory(stivale2_struct_tag_memmap *mem_entry)
{

    size_t length = mem_entry->memmap[mem_entry->entries - 1].length;
    size_t start = mem_entry->memmap[mem_entry->entries - 1].base;

    size_t aligned_start = (start - (start % PAGE_SIZE));
    size_t aligned_length = (length) + PAGE_SIZE - (length % PAGE_SIZE);

    return aligned_start + aligned_length - 1;
}

void init_bitmap_base(stivale2_struct_tag_memmap *mem_entry, size_t targetted_length)
{
    for (uint64_t i = 0; i < mem_entry->entries; i++)
    {
        if (mem_entry->memmap[i].type == STIVALE2_MMAP_USABLE)
        {
            if (mem_entry->memmap[i].length > (((targetted_length) / 8) + (PAGE_SIZE * 2)))
            {

                log("pmm", LOG_INFO, "memory entry used: {}", i);
                log("pmm", LOG_INFO, "total bitmap length: {}", (targetted_length) / 8);
                bitmap_base = mem_entry->memmap[i].base + PAGE_SIZE * 2;

                log("pmm", LOG_INFO, "bitmap addr: {}", bitmap_base);

                pmm_length = (targetted_length);
                return;
            }
        }
    }
}

const char *mem_entry_to_str(uint32_t type)
{
    switch (type)
    {

    case STIVALE2_MMAP_USABLE:
        return "mmap usable";
    case STIVALE2_MMAP_RESERVED:
        return "mmap reserved";
    case STIVALE2_MMAP_ACPI_RECLAIMABLE:
        return "mmap acpi reclaimable";
    case STIVALE2_MMAP_ACPI_NVS:
        return "mmap acpi nvs";
    case STIVALE2_MMAP_BAD_MEMORY:
        return "mmap bad memory";

    case STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE:
        return "mmap bootloader reclaimable";
    case STIVALE2_MMAP_KERNEL_AND_MODULES:
        return "mmap kernel and modules";
    case STIVALE2_MMAP_FRAMEBUFFER:
        return "mmap framebuffer";
    default:
        return "mmap unknown";
    }
}
void init_bitmap_memory_map(stivale2_struct_tag_memmap *mem_entry, bitmap &target)
{

    for (uint64_t i = 0; i < mem_entry->entries; i++)
    {
        log("pmm", LOG_INFO, "entry: {} -- from: {} to: {} -- status: {}", i, mem_entry->memmap[i].base + PAGE_SIZE, (mem_entry->memmap[i].base) + mem_entry->memmap[i].length, mem_entry_to_str(mem_entry->memmap[i].type));

        if (mem_entry->memmap[i].type == STIVALE2_MMAP_USABLE)
        {
            size_t start = ((mem_entry->memmap[i].base) + PAGE_SIZE - ((mem_entry->memmap[i].base) % PAGE_SIZE)) / PAGE_SIZE; // align up
            size_t size = ((mem_entry->memmap[i].length) - ((mem_entry->memmap[i].length) % PAGE_SIZE)) / PAGE_SIZE;          // align down
            target.set_free(start, size - 1);
            available_memory += size;
        }
    }
}

void init_physical_memory(stivale2_struct_tag_memmap *bootdata)
{
    available_memory = 0;

    log("pmm", LOG_DEBUG, "loading pmm");

    uint64_t total_memory_length = get_available_memory(bootdata);

    bitmap_base = 0;

    log("pmm", LOG_INFO, "finding physical memory mem map entry for memory length: {}", total_memory_length);
    init_bitmap_base(bootdata, total_memory_length / PAGE_SIZE);
    log("pmm", LOG_INFO, "pmm length: {}", pmm_length);

    pmm_bitmap = bitmap(reinterpret_cast<uint8_t *>(bitmap_base), pmm_length);

    memset(reinterpret_cast<void *>(bitmap_base), 0xff, (pmm_length) / 8);
    log("pmm", LOG_DEBUG, "loading pmm memory map");

    init_bitmap_memory_map(bootdata, pmm_bitmap);
    log("pmm", LOG_INFO, "pmm length: {}", pmm_length);
    pmm_bitmap.set_used((bitmap_base / PAGE_SIZE), ((pmm_length / 8)) / PAGE_SIZE);
    pmm_bitmap.reset_last_free();

    log("pmm", LOG_INFO, "free memory: {}", available_memory);
}
