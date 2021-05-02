
#ifndef UNIT_TEST
#include "lcxxabi.h"
extern "C"
{

    void *__dso_handle;

    typedef unsigned uarch_t;
    atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
    uarch_t __atexit_func_count = 0;

    int __cxa_atexit(void (*destructor)(void *), void *target, void *dso_handle)
    {

        if (__atexit_func_count >= ATEXIT_MAX_FUNCS)
        {
            return -1;
        };
        __atexit_funcs[__atexit_func_count].destructor = destructor;
        __atexit_funcs[__atexit_func_count].target = target;
        __atexit_funcs[__atexit_func_count].dso_handle = dso_handle;
        __atexit_func_count++;
        return 0;
    };

    void __cxa_finalize(void *destructor)
    {
        uarch_t i = __atexit_func_count;
        if (!destructor)
        {

            while (i--)
            {
                if (__atexit_funcs[i].destructor)
                {

                    (*__atexit_funcs[i].destructor)(__atexit_funcs[i].target);
                };
            };
            return;
        };

        while (i--)
        {
            if (__atexit_funcs[i].destructor == destructor)
            {
                (*__atexit_funcs[i].destructor)(__atexit_funcs[i].target);
                __atexit_funcs[i].destructor = 0;
            };
        };
    };
}
namespace __cxxabiv1
{
    extern "C" int __cxa_guard_acquire(__guard *g)
    {
        return !*(char *)(g);
    }

    extern "C" void __cxa_guard_release(__guard *g)
    {
        *(char *)g = 1;
    }

    extern "C" void __cxa_guard_abort(__guard *)
    {
        // not implemented as the __cxa_guard can't throw for the moment
    }
} // namespace __cxxabiv1

#endif
