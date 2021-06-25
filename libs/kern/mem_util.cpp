#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <string.h>
namespace sys
{

    void *pmm_malloc(size_t length)
    {
        return sys::sys$alloc(length, 0);
    }
    void *pmm_malloc_shared(size_t length)
    {
        return sys::sys$alloc((length / 4096)+1, SYS_ALLOC_SHARED);
    }
    uint64_t pmm_free(void *addr, size_t length)
    {
        return sys::sys$free((uint64_t)addr, length);
    }
} // namespace sys
