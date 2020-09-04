
#pragma once
#include <arch/mem/physical.h>
#include <arch/mem/virtual.h>
#include <arch/process.h>
#include <stdint.h>
/** \defgroup ALLOCHOOKS liballoc hooks
 *
 * These are the OS specific functions which need to
 * be implemented on any platform that the library
 * is expected to work on.
 */

/** @{ */
#define NULL 0
typedef long size_t;

// If we are told to not define our own size_t, then we skip the define.
//#define _HAVE_UINTPTR_T
// typedef	unsigned long	uintptr_t;

// This lets you prefix malloc and friends
#define PREFIX(func) func

#ifdef __cplusplus
extern "C"
{
#endif

    /** This function is supposed to lock the memory data structures. It
 * could be as simple as disabling interrupts or acquiring a spinlock.
 * It's up to you to decide.
 *
 * \return 0 if the lock was acquired successfully. Anything else is
 * failure.
 */
    inline int liballoc_lock()
    {
        lock_process();
        return 0;
    };

    /** This function unlocks what was previously locked by the liballoc_lock
 * function.  If it disabled interrupts, it enables interrupts. If it
 * had acquiried a spinlock, it releases the spinlock. etc.
 *
 * \return 0 if the lock was successfully released.
 */
    inline int liballoc_unlock()
    {
        unlock_process();
        return 0;
    }

    /** This is the hook into the local system which allocates pages. It
 * accepts an integer parameter which is the number of pages
 * required.  The page size was set up in the liballoc_init function.
 *
 * \return NULL if the pages were not allocated.
 * \return A pointer to the allocated memory.
 */
    inline void *liballoc_alloc(int d)
    { // alloc x page

        return pmm_alloc(d);
    }

    /** This frees previously allocated memory. The void* parameter passed
 * to the function is the exact same value returned from a previous
 * liballoc_alloc call.
 *
 * The integer value is the number of pages to free.
 *
 * \return 0 if the memory was successfully freed.
 */
    inline int liballoc_free(void *addr, int d)
    {
        pmm_free(addr, d);
        return 0;
    }

    extern void *PREFIX(malloc)(size_t);          ///< The standard function.
    extern void *PREFIX(realloc)(void *, size_t); ///< The standard function.
    extern void *PREFIX(calloc)(size_t, size_t);  ///< The standard function.
    extern void PREFIX(free)(void *);             ///< The standard function.

#ifdef __cplusplus
}
#endif
