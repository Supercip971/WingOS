#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <klib/process_message.h>
#include <klib/syscall.h>
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
