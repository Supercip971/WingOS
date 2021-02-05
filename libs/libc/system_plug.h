#pragma once
#include <stddef.h>
#include <stdint.h>
namespace plug
{

    uintptr_t allocate_page(size_t count);
    bool free_page(uintptr_t addr, size_t count);
    void debug_out(const char *str, size_t length);
} // namespace plug