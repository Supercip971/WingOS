#include <filesystem/userspace_fs.h>
#include <physical.h>
#include <plug/system_plug.h>
#include <syscall.h>
#include <logging.h>

uintptr_t plug_allocate_page(size_t count)
{

    return reinterpret_cast<uintptr_t>(get_mem_addr(pmm_alloc(count)));
}
int plug_free_page(uintptr_t addr, size_t count)
{

    pmm_free(reinterpret_cast<void *>(get_rmem_addr(addr)), count);
    return true;
}
void plug_debug_out(const char *str, size_t length)
{
    debug_device *dev = find_device<debug_device>();
    dev->echo_out(str, length);
}

int plug_open(const char *path_name, int flags, int mode)
{
    return fs_open(path_name, flags, mode);
}
int plug_close(int fd)
{
    return fs_close(fd);
}
size_t plug_lseek(int fd, size_t offset, int whence)
{
    return fs_lseek(fd, offset, whence);
}
size_t plug_read(int fd, void *buffer, size_t count)
{
    return fs_read(fd, buffer, count);
}
void plug_exit(int s)
{
    log("plug", LOG_ERROR, "trying to call plug exit({}) in the kernel", s);
}
