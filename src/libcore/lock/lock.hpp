#pragma once

#include "arch/generic/instruction.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"





namespace core
{

class Lock
{

    volatile int _locked = 0;

    bool try_acquire()
    {
        int expected = 0;
        return __atomic_compare_exchange_n(
            &_locked, &expected, 1, false,
            __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
    }

public:
    bool try_lock()
    {
        return try_acquire();
    }

    bool view_locked()
    {
        return __atomic_load_n(&_locked, __ATOMIC_ACQUIRE);
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
        __atomic_store_n(&_locked, 0, __ATOMIC_RELEASE);
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

} // namespace core

#define lock_scope$(lock) core::CtxLocker _lock(lock)
