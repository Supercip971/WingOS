#ifndef LCXXABI_H
#define LCXXABI_H
#ifndef UNIT_TEST
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
#endif
#endif // LCXXABI_H
