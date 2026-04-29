#pragma once

#include "iol/lock_context.h"

#include "arch/generic/instruction.hpp"
#include "libcore/atomic.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"

namespace core
{

template <bool _Critical = false>
class _Lock : public NoCopy
{

public:
    core::Atomic<int> _locked = {};
    core::Atomic<int> _secure_ctx = {};

    _Lock() : _locked(0), _secure_ctx(0) {}

    _Lock(_Lock &&v)
    {
        _locked.store(v._locked.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
        _secure_ctx.store(v._secure_ctx.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
    }

    _Lock &operator=(_Lock &&v)
    {
        _locked.store(v._locked.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
        _secure_ctx.store(v._secure_ctx.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
        return *this;
    }

    ~_Lock()
    {
        _locked.store(0, core::MemoryOrder::Relaxed);
    }

    void force_unlock()
    {
        _locked.store(0, core::MemoryOrder::Release);
    }
    bool try_acquire()
    {
        int expected = 0;

        if constexpr (_Critical)
        {

            int ctx = enter_critical_context();

            auto v = _locked.compare_exchange_weak(expected, 1, core::MemoryOrder::Acquire, core::MemoryOrder::Relaxed);

            if (v)
            {
                _secure_ctx.store(ctx);
            }
            else
            {
                exit_critical_context(ctx);
            }

            __atomic_thread_fence(__ATOMIC_SEQ_CST);
            return v;
        }
        else
        {
            auto v = _locked.compare_exchange_weak(expected, 1, core::MemoryOrder::Acquire, core::MemoryOrder::Relaxed);

            __atomic_thread_fence(__ATOMIC_SEQ_CST);

            return v;
        }
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

        int _retry = 1 << 30;
        while (!try_acquire())
        {
            arch::pause();
            if (_retry-- == 0)
            {
                unreachable$();
            }
        }
    }

    bool retry_try_lock(long retry = 5)
    {
        while (!try_acquire())
        {
            arch::pause();
            if (retry-- == 0)
            {
                return false;
            }
        }

        return true;
    }

    void release()
    {

        if (_locked.load(core::MemoryOrder::Acquire) == 0)
        {
            unreachable$();
            return;
        }
        __atomic_thread_fence(__ATOMIC_SEQ_CST);

        auto ctx = _secure_ctx.load(core::MemoryOrder::Acquire);

        _locked.store(0, core::MemoryOrder::Release);
        if constexpr (_Critical)
        {
            exit_critical_context(ctx);
        }
    }
};

using Lock = _Lock<false>;
using LockCritical = _Lock<true>;

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
