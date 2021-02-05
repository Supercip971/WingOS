#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <stddef.h>
#include <system_plug.h>
namespace plug
{

    uintptr_t allocate_page(size_t count)
    {
        return reinterpret_cast<uintptr_t>(sys::pmm_malloc(count));
    }
    bool free_page(uintptr_t addr, size_t count)
    {
        sys::pmm_free(reinterpret_cast<void *>(count), count);
        return true;
    }
    void debug_out(const char *str, size_t length)
    {
        sys::write_console(str, length);
    }
} // namespace plug