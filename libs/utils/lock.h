#ifndef LOCK_H
#define LOCK_H
#include <stddef.h>
#include <stdint.h>
namespace wos
{
    enum lock_state
    {
        LOCK_LOCKED = true,
        LOCK_FREE = false,
    };

    class lock_type
    {
        bool raw;

    public:
        lock_type() : raw(LOCK_FREE){

                      }; // by default the lock is free
        bool is_locked() const
        {
            bool res;
            __atomic_load(&raw, &res, __ATOMIC_SEQ_CST);
            return res;
        }

        void lock()
        {
            while (!__sync_bool_compare_and_swap(&raw, LOCK_FREE, LOCK_LOCKED))
            {
                asm volatile("pause"); // try to not burn the cpu
            }
            __sync_synchronize();
        };
        void unlock()
        {
            __sync_synchronize();

            __atomic_store_n(&raw, LOCK_FREE, __ATOMIC_SEQ_CST);
            raw = LOCK_FREE;
        };
    };
} // namespace wos

#endif // LOCK_H
