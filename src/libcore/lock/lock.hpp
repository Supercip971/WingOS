#pragma once

#include "arch/generic/instruction.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"


#include <atomic>




namespace core
{

class Lock : public NoCopy
{


public:
    std::atomic<int> _locked;


    Lock() : _locked(0) {}

    Lock(Lock&& v) {
        _locked.store(v._locked.load(std::memory_order_acquire), std::memory_order_relaxed);
    }

    Lock& operator=(Lock&& v) {
        _locked.store(v._locked.load(std::memory_order_acquire), std::memory_order_relaxed);
        return *this;
    }

    ~Lock() {
        _locked.store(0, std::memory_order_relaxed);
    }

    bool try_acquire()
    {
        int expected = 0;
        return _locked.compare_exchange_strong(expected, 1, std::memory_order_acquire, std::memory_order_relaxed);
    }

    bool try_lock()
    {
        return try_acquire();
    }

    bool view_locked()
    {
        return _locked.load(std::memory_order_acquire);
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
        _locked.store(0, std::memory_order_release);
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
