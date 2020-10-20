#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <arch/mem/physical.h>
#define MM_BLOCK_SIZE 4096
#define MM_BIG_BLOCK_SIZE 536870912
struct memory_map_children
{
    uint64_t length;
    bool is_free;
    uint16_t code = 0xf2ee;
    memory_map_children *next;
} __attribute__((packed));

void *malloc(uint64_t length);
void free(void *addr);

void dump_memory();
inline void *realloc(void *, uint64_t)
{
    while (true)
    {
    }
};
inline void *calloc(uint64_t, uint64_t)
{

    while (true)
    {
    }
}
#endif // MEMORY_MANAGER_H
