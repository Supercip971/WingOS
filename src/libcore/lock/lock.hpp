#pragma once

#include "iol/lock_context.h"
#include "arch/generic/instruction.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"
#include "libcore/atomic.hpp"




namespace core
{

class Lock : public NoCopy
{



public:
    core::Atomic<int> _locked = {};
    core::Atomic<int> _secure_ctx = {};

    Lock() : _locked(0), _secure_ctx(0) {}

    Lock(Lock&& v) {
        _locked.store(v._locked.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
        _secure_ctx.store(v._secure_ctx.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
    }

    Lock& operator=(Lock&& v) {
        _locked.store(v._locked.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
        _secure_ctx.store(v._secure_ctx.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
        return *this;
    }

    ~Lock() {
        _locked.store(0, core::MemoryOrder::Relaxed);

    }

    bool try_acquire()
    {
        int expected = 0;


        int ctx = enter_critical_context();

        auto v = _locked.compare_exchange_strong(expected, 1, core::MemoryOrder::Acquire, core::MemoryOrder::Relaxed);

        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        if(v)
        {
            _secure_ctx.store(ctx);
        }
        else
        {
            exit_critical_context(ctx);
        }

        return v;
    }

    bool try_lock()
    {
        return try_acquire();
    }

    bool view_locked()
    {
        return _locked.load(core::MemoryOrder::Acquire);
    }

    void lock()
    {
        while (!try_acquire())
        {
            arch::pause();
        }
    }


    bool retry_try_lock(long retry = 100000)
    {
        while (!try_acquire())
        {
            arch::pause();
            if(retry-- == 0)
            {
                return false;
            }
        }

        return true;
    }

    void release()
    {
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        _locked.store(0, core::MemoryOrder::Release);
        exit_critical_context(_secure_ctx.load(core::MemoryOrder::Acquire));

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
