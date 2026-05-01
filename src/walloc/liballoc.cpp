#include "liballoc.h"
#include <stddef.h>
#include <stdint.h>

#include "iol/mem.h"
#include "math/align.hpp"

#define ALLOC_PAGE_SIZE 4096

#define ALLOC_MAGIC 0xDEADEEDEAD
struct WingosAllocNode
{
    unsigned long magic;
    size_t allocated_size;
    size_t page_count;
    uint64_t _padding;
};

void *PREFIX(malloc)(size_t len)
{
    size_t tlen = math::alignUp<size_t>(len + sizeof(WingosAllocNode), ALLOC_PAGE_SIZE);

    liballoc_lock();
    WingosAllocNode *n = (WingosAllocNode *)liballoc_alloc(tlen / ALLOC_PAGE_SIZE);
    liballoc_unlock();

    n->allocated_size = len + sizeof(WingosAllocNode);
    n->page_count = tlen / ALLOC_PAGE_SIZE;

    n->magic = ALLOC_MAGIC;

    return (void *)((char *)n + sizeof(WingosAllocNode));
}

void PREFIX(free)(void *ptr)
{

    if (ptr == nullptr)
    {
        return;
    }
    WingosAllocNode *n = (WingosAllocNode *)((char *)ptr - sizeof(WingosAllocNode));

    if (n->magic != ALLOC_MAGIC)
    {
        unreachable$();
    }

    liballoc_lock();
    liballoc_free(n, n->page_count);
    liballoc_unlock();
}

void *PREFIX(realloc)(void *ptr, size_t len)
{

    if (ptr == nullptr)
    {
        return kmalloc(len);
    }

    WingosAllocNode *n = (WingosAllocNode *)((char *)ptr - sizeof(WingosAllocNode));
    if (len + sizeof(WingosAllocNode) < n->allocated_size)
    {
        n->allocated_size = len + sizeof(WingosAllocNode);
        return ptr;
    }

    if (len + sizeof(WingosAllocNode) < n->page_count * ALLOC_PAGE_SIZE)
    {
        n->allocated_size = len + sizeof(WingosAllocNode);
        return ptr;
    }

    size_t old_size = n->allocated_size - sizeof(WingosAllocNode);

    void *res = kmalloc(len);

    uint8_t *targ = (uint8_t *)res;
    uint8_t *from = (uint8_t *)ptr;

    for (size_t i = 0; i < old_size; i++)
    {
        targ[i] = from[i];
    }

    kfree(ptr);

    return (void *)targ;
}

void *PREFIX(calloc)(size_t n, size_t size)
{

    void *a = kmalloc(n * size);

    for (size_t i = 0; i < n * size; i++)
    {
        ((uint8_t *)a)[i] = 0;
    }

    return a;
}
