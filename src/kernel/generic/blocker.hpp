#pragma once 


#include <stdint.h>

#include <stddef.h>
#include <libcore/lock/lock.hpp>
struct ReceivedIpcMessage;
namespace kernel 
{

    struct BlockMutex 
    {
        core::Lock lock = {};

        // if a task block a mutex, then it release it, then relock it but another task wait on the old one,
        // acquire_uid will change 
        size_t acquire_uid = 0;
        

        bool mutex_acquire()
        {
            return lock.try_lock();
        }

        bool mutex_release()
        {
            if(lock.try_lock())
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

    struct BlockEvent 
    {
        enum class Type
        {
            NONE,
            SLEEP,
            MUTEX
        };
        bool resolved;

        Type type = Type::NONE;
        
        uintptr_t id = 0;

        union {
            long dt;
            BlockMutex* mtx;
        };

        bool is_liberated_async()
        {
            switch(type)
            {
                case Type::MUTEX:
                {
                    bool v = mtx->mutex_value();
                        
                    if(mtx->acquire_uid != id)
                    {
                        return true;
                    }
                    return !v;
                }
                case Type::NONE:
                {
                    return true;
                }
                default: 
                {
                    return false;
                }
            }
 
        }
        bool is_liberated()
        {
            switch(type)
            {
                case Type::MUTEX:
                {
                    if(mtx->mutex_value())
                    {
                        return false;
                    }
                    return true;
                }
                case Type::NONE:
                {
                    return true;
                }
                default: 
                {
                    return false;
                }
            }
        }
    };
    BlockEvent create_block(BlockEvent::Type type, uintptr_t data=  0);

    BlockEvent create_mutex_block(BlockMutex* msg);


}