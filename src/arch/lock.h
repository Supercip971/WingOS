#pragma once
#include <stdint.h>
struct lock_type
{
    uint32_t data;

    const char *file;
    // you can add more data
};

extern "C" void asm_spinlock_lock(volatile uint32_t *lock);
extern "C" void asm_spinlock_unlock(volatile uint32_t *lock);

// we put define so we can do

#define lock(a)         \
    a->file = __FILE__; \
    asm_spinlock_lock(&a->data);

#define unlock(a) \
    asm_spinlock_unlock(&a->data);
