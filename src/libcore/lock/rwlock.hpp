#pragma once

#include <stdatomic.h>

#include "arch/generic/instruction.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/type-utils.hpp"
namespace core
{

/* The RWLock class is a reader-writer lock that allows multiple readers or a single writer to access a resource.
 * It can only write if there is no-one reading.
 *
 *
 */
class RWLock
{
    Lock _access_lock = Lock();
    _Atomic int _readers = 0;
    _Atomic int _writers = 0;
    _Atomic int _waiters = 0;

public:
    RWLock() = default;

    bool write_acquire()
    {
        bool success = false;

        _access_lock.lock();
        _waiters += 1;
        _access_lock.release();
        while (true)
        {
            _access_lock.lock();
            if (_readers == 0 && _writers == 0)
            {
                _writers += 1;
                success = true;
                _waiters -= 1;
                _access_lock.release();
                break;
            }
            _access_lock.release();
            atomic_thread_fence(memory_order_seq_cst);
            arch::pause();
        }
        return success;
    }

    void write_release()
    {
        _access_lock.lock();
        _writers -= 1;
        _access_lock.release();
    }
    bool read_acquire()
    {
        bool success = false;
        while (true)
        {
            _access_lock.lock();
            if (_writers == 0 && _waiters == 0)
            {
                _readers += 1;

                success = true;

                _access_lock.release();
                break;
            }
            _access_lock.release();
            atomic_thread_fence(memory_order_seq_cst);
            arch::pause();
        }
        return success;
    }
    void read_release()
    {
        _access_lock.lock();
        _readers -= 1;
        _access_lock.release();
    }
};

class ReadCtxLocker : public NoCopy, NoMove
{
    RWLock &_lock;

public:
    ReadCtxLocker(RWLock &lock) : _lock(lock)
    {
        _lock.read_acquire();
    }

    ~ReadCtxLocker()
    {
        _lock.read_release();
    }
};
class WriteCtxLocker : public NoCopy, NoMove
{
    RWLock &_lock;

public:
    WriteCtxLocker(RWLock &lock) : _lock(lock)
    {
        _lock.write_acquire();
    }

    ~WriteCtxLocker()
    {
        _lock.write_release();
    }
};

#define lock_scope_reader$(lock) ReadCtxLocker _rlock(lock)

#define lock_scope_writer$(lock) WriteCtxLocker _wlock(lock)

} // namespace core