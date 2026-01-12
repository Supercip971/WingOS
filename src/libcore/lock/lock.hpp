#pragma once

#include "arch/generic/instruction.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"
#include "libcore/atomic.hpp"




namespace core
{

class Lock : public NoCopy
{


public:
    core::Atomic<int> _locked;


    Lock() : _locked(0) {}

    Lock(Lock&& v) {
        _locked.store(v._locked.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
    }

    Lock& operator=(Lock&& v) {
        _locked.store(v._locked.load(core::MemoryOrder::Acquire), core::MemoryOrder::Relaxed);
        return *this;
    }

    ~Lock() {
        _locked.store(0, core::MemoryOrder::Relaxed);
    }

    bool try_acquire()
    {
        int expected = 0;
        
        return _locked.compare_exchange_strong(expected, 1, core::MemoryOrder::Acquire, core::MemoryOrder::Relaxed);
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

    void release()
    {
        _locked.store(0, core::MemoryOrder::Release);
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