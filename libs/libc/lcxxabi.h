#ifndef LCXXABI_H
#define LCXXABI_H
#ifndef UNIT_TEST
#include <utils/lock.h>
extern "C"
{
#define ATEXIT_MAX_FUNCS 128

    typedef unsigned uarch_t;

    struct atexit_func_entry_t
    {
        void (*destructor)(void *);
        void *target;
        void *dso_handle;
    };

    int __cxa_atexit(void (*destructor)(void *), void *target, void *dso_handle);
    void __cxa_finalize(void *targ);
}
namespace __cxxabiv1
{
    __extension__ typedef int __guard __attribute__((mode(__DI__)));
    extern "C" int __cxa_guard_acquire(__guard *);
    extern "C" void __cxa_guard_release(__guard *);
    extern "C" void __cxa_guard_abort(__guard *);

} // namespace __cxxabiv1
#endif
#endif // LCXXABI_H
