#include <kernel/generic/mem.hpp>
#include <liballoc/liballoc.h>

#include "hw/mem/addr_space.hpp"

#include "kernel/generic/pmm.hpp"
#include "libcore/lock/lock.hpp"


core::Lock _liballoc_lock = core::Lock();

extern "C" int liballoc_lock()
{
    _liballoc_lock.lock();
    // FIXME: implement
    return 0;
}
/** This function unlocks what was previously locked by the liballoc_lock
 * function.  If it disabled interrupts, it enables interrupts. If it
 * had acquiried a spinlock, it releases the spinlock. etc.
 *
 * \return 0 if the lock was successfully released.
 */
extern "C" int liballoc_unlock()
{
    _liballoc_lock.release();
    // FIXME: implement
    return 0;
}
/** This is the hook into the local system which allocates pages. It
 * accepts an integer parameter which is the number of pages
 * required.  The page size was set up in the liballoc_init function.
 *
 * \return NULL if the pages were not allocated.
 * \return A pointer to the allocated memory.
 */
extern "C" void *liballoc_alloc(size_t l)
{
    void *res = toVirt(Pmm::the().allocate(l).unwrap());
    return res;
}

/** This frees previously allocated memory. The void* parameter passed
 * to the function is the exact same value returned from a previous
 * liballoc_alloc call.
 *
 * The integer value is the number of pages to free.
 *
 * \return 0 if the memory was successfully freed.
 */
extern "C" int liballoc_free(void *ptr, size_t l)
{

    auto v = Pmm::the().release(toPhys((uintptr_t)ptr), l);
    return 0;
}