#include "memory_manager.h"
#include <arch/lock.h>
#include <arch/process.h>
#include <device/local_data.h>
#include <stddef.h>
memory_map_children *heap = nullptr;
uint64_t heap_length;
// so this work with 512mo of data each time
lock_type memory_lock = {0};
void init_mm()
{
    log("memory manager", LOG_DEBUG) << "loading mm";
    heap = reinterpret_cast<memory_map_children *>(pmm_alloc_zero(MM_BIG_BLOCK_SIZE / 4096));
    heap->code = 0xf2ee;
    heap->length = MM_BIG_BLOCK_SIZE - sizeof(memory_map_children);
    heap->is_free = true;
    heap->next = nullptr;
}
void *addr_from_header(memory_map_children *target)
{
    return reinterpret_cast<void *>(reinterpret_cast<uint64_t>(target) + sizeof(memory_map_children));
}
void increase_mmap()
{
    log("memory manager", LOG_INFO) << "increasing memory manager map";
    memory_map_children *current = heap;
    for (uint64_t i = 0; current != nullptr; i++)
    {
        current = current->next;
        if (current->next == nullptr)
        {

            current->next = reinterpret_cast<memory_map_children *>(pmm_alloc_zero(MM_BIG_BLOCK_SIZE / 4096));
            current->next->code = 0xf2ee;
            current->next->length = MM_BIG_BLOCK_SIZE - sizeof(memory_map_children);
            current->next->is_free = true;
            current->next->next = nullptr;
            return;
        }
    }
}
memory_map_children *last_free = nullptr;
void insert_new_mmap_child(memory_map_children *target, uint64_t length)
{
    uint64_t t = reinterpret_cast<uint64_t>(addr_from_header(target));
    t += length;
    uint64_t previous_next = reinterpret_cast<uint64_t>(target->next);
    target->next = reinterpret_cast<memory_map_children *>(t);
    target->next->next = reinterpret_cast<memory_map_children *>(previous_next);
    target->next->length = target->length - (length + sizeof(memory_map_children));
    target->length = length;
    target->next->code = 0xf2ee;
    target->next->is_free = true;
    if (last_free == nullptr)
    {
        last_free = target->next;
    }
}

void dump_memory()
{
    lock(&memory_lock);
    memory_map_children *current = heap;
    for (uint64_t i = 0; current != nullptr; i++)
    {
        log("mem manager", LOG_DEBUG) << "entry : " << i;
        log("mem manager", LOG_INFO) << "size : " << current->length;
        log("mem manager", LOG_INFO) << "addr : " << reinterpret_cast<uint64_t>(addr_from_header(current));
        log("mem manager", LOG_INFO) << "is_free : " << current->is_free;
        log("mem manager", LOG_INFO) << "code : " << current->code;
        current = current->next;
    }
    unlock(&memory_lock);
}
void check_for_fusion(uint64_t length)
{
    memory_map_children *current = heap;
    const uint64_t targeted_length = length + sizeof(memory_map_children);
    for (uint64_t i = 0; current != nullptr; current = current->next)
    {
        if (current->is_free != true || current->next == nullptr)
        {
            continue;
        }
        if ((current->next->is_free != true))
        {
            continue;
        }
        const uint64_t two_block_length = current->next->length + current->length;
        if ((two_block_length > targeted_length) && (current->length < targeted_length))
        {
            current->length += current->next->length;
            current->length += sizeof(memory_map_children);
            current->next = current->next->next;
            last_free = current;
            return;
        }
    }
}
void *malloc(uint64_t length)
{
    lock(&memory_lock);

    if (length < 16)
    {
        length = 16;
    }
    if (heap == nullptr)
    {
        init_mm();
    }
    check_for_fusion(length);
    memory_map_children *current = heap;
    if (last_free != nullptr)
    {
        current = last_free;
    }
    for (uint64_t i = 0; current != nullptr; i++)
    {
        if (current->is_free == true)
        {
            if (current->length > length + sizeof(memory_map_children))
            {

                current->is_free = false;
                insert_new_mmap_child(current, length);
                current->length = length;
                current->code = 0xf2ee;

                unlock(&memory_lock);

                return addr_from_header(current);
            }
            else if (current->length == length)
            {

                current->is_free = false;
                current->code = 0xf2ee;
                unlock(&memory_lock);

                return addr_from_header(current);
            }
            else if (current->length > length && length + sizeof(memory_map_children) > current->length)
            {

                current->is_free = false;
                current->code = 0xf2ee;
                unlock(&memory_lock);

                return addr_from_header(current);
            }
        }
        current = current->next;
    }
    if (last_free != nullptr)
    {
        last_free = nullptr;
    }
    else
    {

        increase_mmap();
    }
    unlock(&memory_lock);
    return malloc(length);
}
void free(void *addr)
{
    lock(&memory_lock);
    memory_map_children *current = reinterpret_cast<memory_map_children *>(reinterpret_cast<uint64_t>(addr) - sizeof(memory_map_children));
    if (current->code != 0xf2ee)
    {
        log("memory manager", LOG_ERROR) << "trying to free an invalid address" << current->code;
        return;
    }
    else if (current->is_free == true)
    {
        log("memory manager", LOG_ERROR) << "address is already free";
        return;
    }
    last_free = current;
    current->is_free = true;
    unlock(&memory_lock);
}
