#include <liballoc/liballoc.h>


// allocate linux pages
#include <sys/mman.h>

extern "C" int liballoc_lock()
{
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
extern "C" void* liballoc_alloc(size_t l)
{
    return mmap(0, l * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
}

/** This frees previously allocated memory. The void* parameter passed
 * to the function is the exact same value returned from a previous
 * liballoc_alloc call.
 *
 * The integer value is the number of pages to free.
 *
 * \return 0 if the memory was successfully freed.
 */
extern "C" int liballoc_free(void* ptr,size_t l)
{
    munmap(ptr, l * 4096);
    return 0;
}