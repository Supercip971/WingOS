#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/syscall.h>
#include <plug/system_plug.h>
#include <stddef.h>

uintptr_t plug_allocate_page(size_t count)
{
    return reinterpret_cast<uintptr_t>(sys::sys$alloc(count, 0));
}
int plug_free_page(uintptr_t addr, size_t count)
{
    return sys::sys$free(addr, count);
}
void plug_debug_out(const char *str, size_t length)
{
    sys::write_console(str, length);
}

int plug_open(const char *path_name, int flags, int mode)
{
    return sys::sys$open(path_name, flags, mode);
}
int plug_close(int fd)
{
    return sys::sys$close(fd);
}
size_t plug_lseek(int fd, size_t offset, int whence)
{
    return sys::sys$lseek(fd, offset, whence);
}
size_t plug_read(int fd, void *buffer, size_t count)
{
    return sys::sys$read(fd, buffer, count);
}

void plug_exit(int s)
{
    sys::sys$exit(s);
}
