#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <arch/mem/physical.h>
#define MM_BLOCK_SIZE 4096
#define MM_BIG_BLOCK_SIZE 16777216
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
void *realloc(void *target, uint64_t length);
void *calloc(uint64_t nmemb, uint64_t size);
void *operator new(uint64_t size);
void *operator new[](uint64_t size);
void operator delete(void *p);
void operator delete[](void *p);

#endif // MEMORY_MANAGER_H
