#pragma once

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
    protected:
    Lock _access_lock = Lock();

    std::atomic<int> _readers = 0;
    std::atomic<int> _writers = 0;
    std::atomic<int> _waiters = 0;

public:
    RWLock() = default;


    void full_reset()
    {
        _readers = 0;
        _waiters = 0;
        _writers= 0;
        _access_lock.release();
    }

    bool try_write_acquire()
    {
        bool success = false;

        _access_lock.lock();
        _waiters += 1;
        _access_lock.release();
        int retry = 20000;
        while (true)
        {
            _access_lock.lock();
            if (_readers == 0 && _writers == 0)
            {
                _writers += 1;
                success = true;
                _waiters -= 1;
                _access_lock.release();
                __atomic_thread_fence(__ATOMIC_SEQ_CST);
                break;
            }
            _access_lock.release();
            __atomic_thread_fence(__ATOMIC_SEQ_CST);

            arch::pause();
            retry--;
            if(retry==0)
            {
                _access_lock.lock();
                _waiters -= 1;
                _access_lock.release();
                __atomic_thread_fence(__ATOMIC_SEQ_CST);


                return false;
            }

        }
        return success;

    }
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
            arch::pause();
        }
        return success;
    }

    // convert 1 writer into a reader
    void release_mutability()
    {
        _access_lock.lock();
        _writers -= 1;
        _readers += 1;
        _access_lock.release();
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
