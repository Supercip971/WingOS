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
class ram_file
{ // ram_file are file that are stored in the ram such as stdout stdin ...
public:
    char *buffer = nullptr;
    int size = 0;
    virtual const char *get_npath() { return "invalid ram file"; };
    virtual int read(void *buffer, size_t offset, size_t count);
    virtual int write(const void *buffer, size_t offset, size_t count);
};
class std_zero_file : public ram_file
{
public:
    virtual const char *get_npath() { return "/dev/zero"; };
    virtual int read(void *buffer, size_t offset, size_t count);
    virtual int write(const void *buffer, size_t offset, size_t count);
};

class std_stdbuf_file : public ram_file
{
public:
    virtual int increase_buffer(int new_size);
    virtual int read(void *buffer, size_t offset, size_t count);
    virtual int write(const void *buffer, size_t offset, size_t count);
};

class std_stdin_file : public std_stdbuf_file
{
public:
    virtual const char *get_npath() { return "/dev/stdin"; };
};
class std_stderr_file : public std_stdbuf_file
{
public:
    virtual const char *get_npath() { return "/dev/stderr"; };
};
class std_stdout_file : public std_stdbuf_file
{
public:
    virtual const char *get_npath() { return "/dev/stdout"; };
};

struct per_process_userspace_fs
{
    static const int ram_files_count = 16;
    ram_file *ram_files[ram_files_count]; // in this we have stdio / stdin / stderr ...
};


size_t fs_read(int fd, void *buffer, size_t count);
size_t fs_write(int fd, const void *buffer, size_t count);
int fs_open(const char *path_name, int flags, int mode);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);
void init_userspace_fs();
#endif // USER_FS_H
