#ifndef USER_FS_H
#define USER_FS_H
#include <io_device.h>
#include <stddef.h>
#include <stdint.h>
#include <utils/container/wvector.h>

class vfile
{ // vfile are file that are stored in the ram such as stdout stdin ...
protected:
    size_t size = 0;

public:
    char *buffer = nullptr;
    virtual size_t get_size() const { return size; };
    virtual const char *get_npath() { return "invalid ram file"; };
    virtual size_t read(void *dbuffer, size_t offset, size_t count);
    virtual size_t write(const void *dbuffer, size_t offset, size_t count);
};
void add_ram_file(vfile *file);

struct filesystem_file_t
{

    char *path;
    int mode;
    size_t cur;
    uint64_t rpid;
    int state;
    bool can_free_handle;
    class vfile *file;
};

enum file_system_file_state
{
    FS_STATE_ERROR = 0,
    FS_STATE_FREE = 1,
    FS_STATE_USED = 2,
    FS_STATE_RESERVED = 3, // may be used later
};

class dev_keyboard_file : public vfile
{
public:
    virtual const char *get_npath() { return "/dev/keyboard"; };
    virtual size_t read(void *dbuffer, size_t offset, size_t count)
    {

        return find_device<general_keyboard>()->read_key_buffer(dbuffer, offset, count);
    }
    virtual size_t get_size() const
    {

        return find_device<general_keyboard>()->get_key_buffer_size();
    };
};
class dev_mouse_file : public vfile
{
public:
    virtual const char *get_npath() { return "/dev/mouse"; };

    virtual size_t read(void *dbuffer, size_t offset, size_t count)
    {

        return find_device<general_mouse>()->read_mouse_buffer(dbuffer, offset, count);
    }
    virtual size_t get_size() const { return sizeof(mouse_buff_info); };
};

class dev_framebuffer_file : public vfile
{
public:
    virtual const char *get_npath() { return "/dev/framebuffer"; };

    virtual size_t read(void *dbuffer, size_t offset, size_t count)
    {
        return find_device<generic_framebuffer>()->read_buffer(dbuffer, count);
    }
    virtual size_t get_size() const { return sizeof(mouse_buff_info); };
};

class std_zero_file : public vfile
{
public:
    virtual const char *get_npath() { return "/dev/zero"; };
    virtual size_t read(void *dbuffer, size_t offset, size_t count);
    virtual size_t write(const void *dbuffer, size_t offset, size_t count) { return 0; };
};

class std_stdbuf_file : public vfile
{
protected:
    void realocate(size_t new_size);
    size_t logging_pos = 0;
    size_t logging_pos_start = 0;

public:
    virtual size_t read(void *dbuffer, size_t offset, size_t count);
    virtual size_t write(const void *dbuffer, size_t offset, size_t count);
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

class disk_file : public vfile
{
    const char *_path;

public:
    disk_file(const char *path) : vfile()
    {
        _path = path;
    }

    virtual const char *get_npath() { return _path; };
    virtual size_t read(void *dbuffer, size_t offset, size_t count);
    virtual size_t write(const void *dbuffer, size_t offset, size_t count);
    virtual size_t get_size() const;
};

class ram_dir
{
public:
    virtual const char *get_path() { return "invalid path"; };
    virtual vfile *get(const char *msg) { return nullptr; };
};

class process_ramdir : public ram_dir
{
public:
    virtual const char *get_path() { return "/proc/"; };
    virtual vfile *get(const char *msg);
};

struct per_process_userspace_fs
{
    static const int ram_files_count = 16;
    vfile *ram_files[ram_files_count]; // in this we have stdio / stdin / stderr ...
};

bool is_ram_file(const char *path);
vfile *get_ram_file(const char *path);

size_t fs_read(int fd, void *buffer, size_t count);
size_t fs_write(int fd, const void *buffer, size_t count);
int fs_open(const char *path_name, int flags, int mode);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);
void init_userspace_fs();
void init_process_userspace_fs(per_process_userspace_fs &target);
#endif // USER_FS_H
