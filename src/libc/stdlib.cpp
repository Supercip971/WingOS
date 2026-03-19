

#include <stdlib.h>
#include <stddef.h>
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"

namespace core
{
    void dump_vec(uintptr_t _this, uintptr_t _data, bool start)
    {


        (void)_this;
        (void)_data;
        (void)start;
        //log::log$("Vec destructed: {} - {} (begin: {})", (uintptr_t)_this | fmt::FMT_HEX, (uintptr_t)_data | fmt::FMT_HEX, start);
    }
}
__attribute__((weak)) void *operator new(size_t size)
{
    return malloc(size);
}

__attribute__((weak)) void *operator new[](size_t size)
{
    return malloc(size);
}

__attribute__((weak)) void operator delete(void *p) noexcept
{
    free(p);
}

__attribute__((weak)) void operator delete[](void *p) noexcept
{
    free(p);
}

__attribute__((weak)) void operator delete(void *p, size_t) noexcept
{
    free(p);
}

__attribute__((weak)) void operator delete[](void *p, size_t) noexcept
{
    free(p);
}
extern "C" void exit(int code)
{
    (void)code;
    while(true)
    {

    }
}
