#pragma once
#include <arch.h>
#include <stdint.h>
ASM_FUNCTION void asm_spinlock_lock(volatile uint32_t *lock);
ASM_FUNCTION void asm_spinlock_unlock(volatile uint32_t *lock);

#define klock(a)            \
    (a)->file = __FILE__;   \
    (a)->line = __LINE__;   \
    (a)->cantforce = false; \
    asm_spinlock_lock(&((a)->data));

#define kflock(a)          \
    (a)->file = __FILE__;  \
    (a)->line = __LINE__;  \
    (a)->cantforce = true; \
    asm_spinlock_lock(&((a)->data));
#define kunlock(a) \
    asm_spinlock_unlock(&((a)->data));
