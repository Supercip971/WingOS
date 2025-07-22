#pragma once 

#include <stdatomic.h>
namespace core 
{

 

    static inline void atomic_cache_flush()
    {
        // This is a no-op on x86_64, as the cache is flushed automatically
        // on context switches and memory accesses.
        // However, we can use this function to ensure that the compiler does not optimize away
        // any atomic operations that we perform.
        asm volatile("mfence" ::: "memory");
    }

    static inline void atomic_cache_sync()
    {
        // This is a no-op on x86_64, as the cache is flushed automatically
        // on context switches and memory accesses.
        // However, we can use this function to ensure that the compiler does not optimize away
        // any atomic operations that we perform.
        asm volatile("sfence" ::: "memory");
    }
}