#pragma once

#include <stdatomic.h>

#include "arch/generic/instruction.hpp"
#include "libcore/type-utils.hpp"
namespace core
{
class Lock
{

    _Atomic int _locked = false;

    bool try_acquire()
    {
        int expected = false;
        int result = atomic_compare_exchange_strong(&_locked, &expected, true);

        atomic_thread_fence(memory_order_seq_cst);
        return result;
    }

public:
    bool try_lock()
    {
        if (try_acquire())
        {
            return true;
        }
        return false;
    };

    void lock()
    {
        while (!try_acquire())
        {
            atomic_thread_fence(memory_order_seq_cst);
            arch::pause();
        }
    };

    void release()
    {
        atomic_thread_fence(memory_order_seq_cst);
        _locked = false;
    };
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