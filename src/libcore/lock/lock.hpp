#pragma once

#include "arch/generic/instruction.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"

// Use compiler builtins for atomics instead of stdatomic.h
#ifndef __clang__

#    define memory_order_relaxed __ATOMIC_RELAXED
#    define memory_order_acquire __ATOMIC_ACQUIRE
#    define memory_order_release __ATOMIC_RELEASE
#else





#endif

#ifndef __clang__
namespace core
{

class Lock
{

    int _locked = 0;

    bool try_acquire()
    {
        int expected = 0;
        return __atomic_compare_exchange_n(
            &_locked, &expected, 1, false,
            memory_order_acquire, memory_order_relaxed);
    }

public:
    bool try_lock()
    {
        return try_acquire();
    }

    bool view_locked()
    {
        return __atomic_load_n(&_locked, memory_order_acquire);
    }

    void lock()
    {
        while (!try_acquire())
        {
            arch::pause();
        }
    }

    void release()
    {
        __atomic_store_n(&_locked, 0, memory_order_release);
    }
};

class CtxLocker : public NoCopy, NoMove
{
    Lock &_lock;

public:
    CtxLocker(Lock &lock) : _lock(lock)
    {
        _lock.lock();
    }

    ~CtxLocker()
    {
        _lock.release();
    }
};


#else


#    include <stdatomic.h>

#    include "arch/generic/instruction.hpp"
#    include "libcore/type-utils.hpp"
namespace core
{
class Lock
{

    _Atomic(int) _locked = false;

    bool try_acquire()
    {
        int expected = false;
        return atomic_compare_exchange_strong_explicit(
            &_locked, &expected, true,
            memory_order_acquire, memory_order_relaxed);
    }

public:
    bool try_lock()
    {
        return try_acquire();
    }

    bool view_locked()
    {
        return atomic_load_explicit(&_locked, memory_order_acquire);
    }

    void lock()
    {
        while (!try_acquire())
        {
            arch::pause();
        }
    }

    void release()
    {
        atomic_store_explicit(&_locked, false, memory_order_release);
    }
};

class CtxLocker : public NoCopy, NoMove
{
    Lock &_lock;

public:
    CtxLocker(Lock &lock) : _lock(lock)
    {
        _lock.lock();
    }

    ~CtxLocker()
    {
        _lock.release();
    }
};

#endif
#define lock_scope$(lock) core::CtxLocker _lock(lock)

} // namespace core