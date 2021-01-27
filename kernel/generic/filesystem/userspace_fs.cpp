#include "userspace_fs.h"
#include <device/local_data.h>
#include <filesystem/file_system.h>
#include <liballoc.h>
#include <logging.h>
#include <utility.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
enum file_system_file_state
{
    FS_STATE_ERROR = 0,
    FS_STATE_FREE = 1,
    FS_STATE_USED = 2,
    FS_STATE_RESERVED = 3, // may be used later

};
#define MAX_FILE_HANDLE 255
filesystem_file_t fs_handle_table[MAX_FILE_HANDLE];

void init_userspace_fs()
{
    for (int i = 0; i < MAX_FILE_HANDLE; i++)
    {
        fs_handle_table[i].state = FS_STATE_FREE;
    }
}

filesystem_file_t *get_if_valid_handle(int fd, bool check_free = true)
{
    if (fd > MAX_FILE_HANDLE)
    {
        log("fs", LOG_WARNING) << "process " << get_current_cpu_process()->process_name << " trying to use an invalid file descriptor";
        return nullptr;
    }
    else if (fs_handle_table[fd].state != file_system_file_state::FS_STATE_USED && check_free)
    {
        log("fs", LOG_WARNING) << "process " << get_current_cpu_process()->process_name << " trying to use a file descriptor already free";
        return nullptr;
    }
    else if (fs_handle_table[fd].rpid != get_current_cpu_process()->upid && check_free)
    {
        log("fs", LOG_WARNING) << "process " << get_current_cpu_process()->process_name << " trying to use a file descriptor used by another process";
        return nullptr;
    }
    return &fs_handle_table[fd];
}

int get_free_handle()
{
    for (int i = 0; i < MAX_FILE_HANDLE; i++)
    {
        if (fs_handle_table[i].state == file_system_file_state::FS_STATE_FREE)
        {
            return i;
        }
    }
    return -1;
}

int set_free_handle(int fd)
{
    auto handle = get_if_valid_handle(fd);
    if (handle == nullptr)
    {
        log("fs", LOG_WARNING) << "process " << get_current_cpu_process()->process_name << " trying to free an invalid file descriptor";

        return -1;
    }
    handle->state = file_system_file_state::FS_STATE_FREE;
    return fd;
}

int set_used_handle(int fd)
{
    auto handle = get_if_valid_handle(fd, false);

    if (handle == nullptr)
    {
        log("fs", LOG_WARNING) << "process " << get_current_cpu_process()->process_name << " trying to set used an invalid file descriptor";

        return -1;
    }
    handle->state = file_system_file_state::FS_STATE_USED;
    return fd;
}

size_t fs_read(int fd, void *buffer, size_t count)
{
    filesystem_file_t *file = get_if_valid_handle(fd);
    if (file == nullptr)
    {
        log("fs", LOG_ERROR) << "process " << get_current_cpu_process()->process_name << " can't read file";
        return 0;
    }
    auto result = main_fs_system::the()->main_fs()->read_file(file->path, file->cur, count, (uint8_t *)buffer);
    file->cur += result;
    return result;
}

size_t fs_lseek(int fd, size_t offset, int whence)
{

    filesystem_file_t *file = get_if_valid_handle(fd);
    if (file == nullptr)
    {
        log("fs", LOG_ERROR) << "process " << get_current_cpu_process()->process_name << " can't seek file invalid fd";
        return 0;
    }
    if (whence == SEEK_SET)
    {
        file->cur = offset;
    }
    if (whence == SEEK_CUR)
    {
        file->cur += offset;
    }
    if (whence == SEEK_END)
    {
        log("fs", LOG_WARNING) << "process " << get_current_cpu_process()->process_name << " can't seek file: SEEK_END not implemented";

        // unimplemented :^(
    }
    return file->cur;
}

size_t fs_write(int fd, const void *buffer, size_t count)
{
    filesystem_file_t *file = get_if_valid_handle(fd);
    if (file == nullptr)
    {
        log("fs", LOG_ERROR) << "process " << get_current_cpu_process()->process_name << " can't read file";
        return 0;
    }
    return main_fs_system::the()->main_fs()->write_file(file->path, file->cur, count, (uint8_t *)buffer);
}

int fs_open(const char *path_name, int flags, int mode)
{
    int fd = get_free_handle();
    if (fd == -1)
    {
        log("fs", LOG_ERROR) << "process " << get_current_cpu_process()->process_name << " can't get free handle";
        return 0;
    }
    set_used_handle(fd);
    fs_handle_table[fd].rpid = get_current_cpu_process()->upid;
    fs_handle_table[fd].path = path_name;
    fs_handle_table[fd].mode = mode;
    fs_handle_table[fd].cur = 0;
    if (main_fs_system::the()->main_fs()->get_file_length(path_name) == (uint64_t)-1)
    {
        log("fs", LOG_WARNING) << "process " << get_current_cpu_process()->process_name << " trying to open file" << path_name << "but it doesn't exits";

        return -1;
    }
    log("fs", LOG_INFO) << "process " << get_current_cpu_process()->process_name << " open " << path_name;

    return fd;
}

int fs_close(int fd)
{
    auto file = get_if_valid_handle(fd);
    if (file == nullptr)
    {
        log("fs", LOG_ERROR) << "process " << get_current_cpu_process()->process_name << " invalid fd for closing file ";
        return 0;
    }

    set_free_handle(fd);
    log("fs", LOG_INFO) << "process " << get_current_cpu_process()->process_name << " closed " << file->path;
    return 1;
}
