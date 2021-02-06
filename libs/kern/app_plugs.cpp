#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/syscall.h>
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

    int open(const char *path_name, int flags, int mode)
    {
        return sys::sys$open(path_name, flags, mode);
    }
    int close(int fd)
    {
        return sys::sys$close(fd);
    }
    size_t lseek(int fd, size_t offset, int whence)
    {
        return sys::sys$lseek(fd, offset, whence);
    }
    size_t read(int fd, void *buffer, size_t count)
    {
        return sys::sys$read(fd, buffer, count);
    }
} // namespace plug
