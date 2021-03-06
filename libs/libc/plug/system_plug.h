#pragma once
#include <stddef.h>
#include <stdint.h>
namespace plug
{
    void init();
    uintptr_t allocate_page(size_t count);
    bool free_page(uintptr_t addr, size_t count);
    void debug_out(const char *str, size_t length);

    int open(const char *path_name, int flags, int mode);
    int close(int fd);
    size_t lseek(int fd, size_t offset, int whence);
    size_t read(int fd, void *buffer, size_t count);
    void exit(int s);
} // namespace plug
