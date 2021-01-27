#ifndef USER_FS_H
#define USER_FS_H
#include <stddef.h>
#include <stdint.h>

struct filesystem_file_t
{
    const char *path;
    int mode;
    size_t cur;
    uint64_t rpid;
    int state;
};

size_t fs_read(int fd, void *buffer, size_t count);
size_t fs_write(int fd, const void *buffer, size_t count);
int fs_open(const char *path_name, int flags, int mode);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);
void init_userspace_fs();
#endif // USER_FS_H
