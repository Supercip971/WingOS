#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
//#include <feather_language_lib/feather.h>
#include <gui/raw_graphic.h>
#include <stdio.h>
struct memory_map_children
{
    uint64_t length;
    bool is_free;
    uint16_t code = 0xf2ee;
    memory_map_children *next;
    bool is_next_contignous;
} __attribute__((packed));

#define MM_BLOCK_SIZE 4096
#define MM_BIG_BLOCK_SIZE 16777216
memory_map_children *heap = nullptr;
void init_mm()
{
    heap = reinterpret_cast<memory_map_children *>(sys::pmm_malloc(MM_BIG_BLOCK_SIZE / 4096));
    heap->code = 0xf2ee;
    heap->length = MM_BIG_BLOCK_SIZE - sizeof(memory_map_children);
    heap->is_free = true;
    heap->next = nullptr;
}

void *addr_from_header(memory_map_children *target)
{
    return reinterpret_cast<void *>(reinterpret_cast<uint64_t>(target) + sizeof(memory_map_children));
}
bool is_next_entry_contignous(memory_map_children *child)
{
    return ((uint64_t)child->length + (uint64_t)addr_from_header(child)) == (uint64_t)child->next;
}
void increase_mmap()
{
    memory_map_children *current = heap;
    for (uint64_t i = 0; current != nullptr; i++)
    {
        if (current->next == nullptr)
        {

            current->next = reinterpret_cast<memory_map_children *>(sys::pmm_malloc(MM_BIG_BLOCK_SIZE / 4096));

            current->next->code = 0xf2ee;
            current->next->length = MM_BIG_BLOCK_SIZE - sizeof(memory_map_children);
            current->next->is_free = true;
            current->next->next = nullptr;

            return;
        }
        current = current->next;
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
    target->next->is_next_contignous = target->is_next_contignous;

    target->is_next_contignous = true;
    if (last_free == nullptr)
    {
        last_free = target->next;
    }
}

void dump_memory()
{
}
void check_for_fusion(uint64_t length)
{
    memory_map_children *current = heap;
    const uint64_t targeted_length = length - sizeof(memory_map_children);
    for (uint64_t i = 0; current != nullptr; i++)
    {
        if (current->is_free != true || current->next == nullptr)
        {
            current = current->next;
            continue;
        }
        if (current->length >= length)
        {
            last_free = current;
            return;
        }
        if (!is_next_entry_contignous(current))
        {
            continue;
        }
        if ((current->next->is_free != true))
        {
            current = current->next;
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
        current = current->next;
    }
}
void *smalloc(uint64_t length)
{

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

                return addr_from_header(current);
            }
            else if (current->length == length)
            {

                current->is_free = false;
                current->code = 0xf2ee;

                return addr_from_header(current);
            }
            else if (current->length > length && length + sizeof(memory_map_children) > current->length)
            {

                current->is_free = false;
                current->code = 0xf2ee;
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
    return smalloc(length);
}
void sfree(void *addr)
{
    memory_map_children *current = reinterpret_cast<memory_map_children *>(reinterpret_cast<uint64_t>(addr) - sizeof(memory_map_children));
    if (current->code != 0xf2ee)
    {
        return;
    }
    else if (current->is_free == true)
    {
        return;
    }
    last_free = current;
    current->is_free = true;
}

void *srealloc(void *target, uint64_t length)
{
    memory_map_children *current = reinterpret_cast<memory_map_children *>(reinterpret_cast<uint64_t>(target) - sizeof(memory_map_children));
    if (current->length >= length)
    {
        return target;
    }

    uint64_t target_length = length - sizeof(memory_map_children);
    if (current->next->is_free == true)
    {
        if (current->next->length + current->length > target_length)
        {
            current->next->is_free = false;
            current->length += current->next->length + sizeof(memory_map_children);
            current->next = current->next->next;
            return addr_from_header(current);
        }
    }
    uint8_t *new_targ = (uint8_t *)smalloc(length);
    uint8_t *from = (uint8_t *)target;
    for (uint64_t i = 0; i < current->length; i++)
    {
        new_targ[i] = from[i];
    }
    sfree(from);
    return new_targ;
}
void *scalloc(uint64_t nmemb, uint64_t size)
{
    const uint64_t clength = nmemb * size;
    uint8_t *result = (uint8_t *)smalloc(nmemb * size);
    for (uint64_t i = 0; i < clength; i++)
    {
        result[i] = 0;
    }
    return result;
}

int main()
{
    init_mm();
    increase_mmap();
    sys::set_current_process_as_a_service("usr_mem_service", true);
    last_free = nullptr;
    heap = nullptr;
    while (true)
    {
        sys::raw_process_message *msg = sys::service_read_current_queue();
        if (msg != 0x0)
        {
            /*
            sys::memory_service_protocol *pr = (sys::memory_service_protocol *)msg->content_address;
            if (pr->request_type == sys::REQUEST_MALLOC)
            {
                msg->response = (uint64_t)smalloc(pr->length);
            }
            if (pr->request_type == sys::REQUEST_FREE)
            {
                msg->response = 1;
                sfree((void *)pr->address);
            }
            if (pr->request_type == sys::REQUEST_REALLOC)
            {
                msg->response = (uint64_t)srealloc((void *)pr->address, pr->length);
            }*/
            msg->has_been_readed = true;
        }
        else
        {
        }
    }
    return 0;
}
