#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    enum IolAllocMemoryFlag
    {
        IOL_ALLOC_MEMORY_FLAG_NONE = 0,

        /* IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE
         * In the case of a physical memory allocation for a legacy
         * device, the memory should be allocated in the lower 4GB.
         * So that the device can access it.
         */
        IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE = 1 << 0,
    };
#ifdef __cplusplus
}
#endif