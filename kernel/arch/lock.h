#pragma once
#include <kernel.h>
#include <stdint.h>
struct lock_type
{
    uint32_t data;

    const char *file;
    uint64_t line;
    bool cantforce;
};

ASM_FUNCTION void asm_spinlock_lock(volatile uint32_t *lock);
ASM_FUNCTION void asm_spinlock_unlock(volatile uint32_t *lock);

#define lock(a)             \
    (a)->file = __FILE__;   \
    (a)->line = __LINE__;   \
    (a)->cantforce = false; \
    asm_spinlock_lock(&((a)->data));

#define flock(a)           \
    (a)->file = __FILE__;  \
    (a)->line = __LINE__;  \
    (a)->cantforce = true; \
    asm_spinlock_lock(&((a)->data));
#define unlock(a) \
    asm_spinlock_unlock(&((a)->data));
