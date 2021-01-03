#pragma once
#include <stddef.h>
#include <stdint.h>
namespace sys
{

    void *pmm_malloc(size_t length);
    uint64_t pmm_free(void *addr, size_t length);
} // namespace sys
