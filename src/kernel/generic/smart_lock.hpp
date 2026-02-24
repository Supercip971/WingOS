#pragma once

#include "libcore/fmt/log.hpp"
#include "libcore/lock/rwlock.hpp"

// a drop in replacement for RWLock that adds some debugging features, such as dumping the state of the lock
class SRWLock : private core::RWLock
{

    std::atomic<const char *>_last_write_acquire_fn = nullptr;
    std::atomic<int> _last_write_acquire_line = 0;


    static constexpr int MAX_READERS = 200;

    volatile int allocated_readers = 0;
    const char* mreaders_file[MAX_READERS] = {};
    int mreaders_line[MAX_READERS] = {};


public:


    bool write_acquire(const char *fn, int line)
    {
        auto v = core::RWLock::try_write_acquire();
        if(!v)
        {
            log::log$("SRWLock: failed to acquire write lock immediately at {}:{}, dumping state before blocking", fn, line);
            dump();
            // Failed to acquire immediately, fall back to regular acquire which will block until it can acquire.
            core::RWLock::write_acquire();
        }

        allocated_readers = 0;
        _last_write_acquire_fn = fn;
        _last_write_acquire_line = line;
        return v;
    }

    bool read_acquire(const char *fn, int line)
    {
        auto v = core::RWLock::read_acquire();

        // Track (best-effort) where readers came from, without going out of bounds.
        int idx = allocated_readers;
        if (idx < 0)
        {
            idx = 0;
        }

        if (idx < MAX_READERS)
        {
            mreaders_file[idx] = fn;
            mreaders_line[idx] = line;
            allocated_readers = idx + 1;
        }
        else
        {
            // Saturate: we prefer keeping earlier entries rather than corrupting memory.
            allocated_readers = MAX_READERS;
        }

        return v;
    }

    bool try_write_acquire(const char *fn, int line)
    {
        auto v = core::RWLock::try_write_acquire();

        if (v)
        {
            _last_write_acquire_fn = fn;
            _last_write_acquire_line = line;
        }
        return v;
    }

    void write_release()
    {
               // Reset reader tracking on writer release so stale entries don't linger forever.
        // This is best-effort debugging info; it does not affect correctness.
        allocated_readers = 0;
        for (int i = 0; i < MAX_READERS; i++)
        {
            mreaders_file[i] = nullptr;
            mreaders_line[i] = 0;
        }

        core::RWLock::write_release();
    }


    void release_mutability()
    {
      //  _last_write_acquire_fn = "released mutability";
      //  _last_write_acquire_line = 0;
            core::RWLock::release_mutability();

    }

    void read_release()
    {
        core::RWLock::read_release();
    }

    void dump()
    {
        const char* state = "unknown";
        int line = -1;
        if(_last_write_acquire_fn == nullptr)
        {
            state = "never acquired";
        }

        else
        {
            state = (const char*)_last_write_acquire_fn;
            line = (int)_last_write_acquire_line;
        }


        int wcount = _writers;
        int rcount = _readers;
        int wwaiters = _waiters;
        log::log$("SRWLock state: {} writer(s), {} reader(s), {} waiting writer(s)", wcount, rcount, wwaiters);
        log::log$("SRWLock dump: last write acquire at {}:{}",core::Str(state), line);

        for(int i = 0; i < allocated_readers; i++)
        {
            if(mreaders_file[i] != nullptr)
            {

                log::log$("  - reader {} at {}:{}", i, mreaders_file[i], mreaders_line[i]);
            }
        }

    }
};


#define srwlock_write_acquire$(lock) (lock).write_acquire(__FILE__, __LINE__)
#define srwlock_try_write_acquire$(lock) (lock).try_write_acquire(__FILE__, __LINE__)
#define srwlock_read_acquire$(lock) (lock).read_acquire(__FILE__, __LINE__)
