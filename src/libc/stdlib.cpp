

#include <stdlib.h>
#include <stddef.h>
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"

namespace __cxxabiv1
{
	/* guard variables */

	/* The ABI requires a 64-bit type.  */
	__extension__ typedef int __guard __attribute__((mode(__DI__)));

	extern "C" int __cxa_guard_acquire (__guard *);
	extern "C" void __cxa_guard_release (__guard *);
	extern "C" void __cxa_guard_abort (__guard *);

	extern "C" int __cxa_guard_acquire (__guard *g)
	{
		return !*(char *)(g);
	}

	extern "C" void __cxa_guard_release (__guard *g)
	{
		*(char *)g = 1;
	}

	extern "C" void __cxa_guard_abort (__guard *)
	{

	}
}

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
