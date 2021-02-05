#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <string.h>
namespace sys
{

    void *pmm_malloc(size_t length)
    {
        return sys::sys$alloc(length);
    }
    uint64_t pmm_free(void *addr, size_t length)
    {
        return sys::sys$free((uint64_t)addr, length);
    }
} // namespace sys
