#pragma once
#include <stddef.h>
#include <stdint.h>
void plug_init();
uintptr_t plug_allocate_page(size_t count);
int plug_free_page(uintptr_t addr, size_t count);
void plug_debug_out(const char *str, size_t length);

int plug_open(const char *path_name, int flags, int mode);
int plug_close(int fd);
size_t plug_lseek(int fd, size_t offset, int whence);
size_t plug_read(int fd, void *buffer, size_t count);
void plug_exit(int s);
