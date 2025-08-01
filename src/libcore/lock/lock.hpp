#pragma once

#include "arch/generic/instruction.hpp"
#include "libcore/type-utils.hpp"
#include <stdatomic.h>
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

#define lock_scope$(lock) core::CtxLocker _lock(lock)

} // namespace core