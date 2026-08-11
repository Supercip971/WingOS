#pragma once

#include <libcore/lock/lock.hpp>
#include <stddef.h>
#include <stdint.h>
struct ReceivedIpcMessage;

namespace kernel
{

struct BlockMutex
{
    fc::Lock lock = {};

    // if a task block a mutex, then it release it, then relock it but another task wait on the old one,
    // acquire_uid will change
    size_t acquire_uid = 0;

    bool mutex_acquire()
    {
        lock.lock();
        return true;
    }

    bool mutex_release()
    {
        if (lock.try_lock())
        {
            // was not locked
            lock.release();
            return false;
        }
        lock.release();
        return true;
    }

    bool mutex_value()
    {
        return lock.view_locked();
    }
};

} // namespace kernel
