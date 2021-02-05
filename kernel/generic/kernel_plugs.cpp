#include <physical.h>
#include <system_plug.h>

namespace plug
{
    uintptr_t allocate_page(size_t count)
    {

        return reinterpret_cast<uintptr_t>(pmm_alloc(count));
    }
    bool free_page(uintptr_t addr, size_t count)
    {

        pmm_free(reinterpret_cast<void *>(addr), count);
        return true;
    }
    void debug_out(const char *str, size_t length)
    {
        debug_device *dev = find_device<debug_device>();
        dev->echo_out(str, length);
    }

} // namespace plug