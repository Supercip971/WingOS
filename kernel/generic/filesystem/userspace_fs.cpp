#include "userspace_fs.h"
#include <device/io_device.h>
#include <device/local_data.h>
#include <filesystem/file_system.h>
#include <logging.h>
#include <stdlib.h>
#include <string.h>
#include <utility.h>
#include <utils/lock.h>
#include <utils/memory/liballoc.h>

utils::lock_type handle_lock;
utils::vector<vfile *> ram_file_list;

void add_ram_file(vfile *file)
{
    log("ram_file", LOG_INFO, "added new general ram file {}", file->get_npath());
    ram_file_list.push_back(file);
}
void remove_ram_file(const char *path)
{
    for (size_t i = 0; i < ram_file_list.size(); i++)
    {
        if (ram_file_list[i]->get_npath() == path)
        {
            ram_file_list.remove(i);
            return;
        }
    }
}
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

size_t disk_file::read(void *dbuffer, size_t offset, size_t count)
{
    auto result = main_fs_system::the()->main_fs()->read_file(_path, offset, count, (uint8_t *)dbuffer);
    return result;
}

size_t disk_file::write(const void *dbuffer, size_t offset, size_t count)
{
    auto result = main_fs_system::the()->main_fs()->write_file(_path, offset, count, (uint8_t *)dbuffer);
    return result;
}

size_t disk_file::get_size() const
{
    auto result = main_fs_system::the()->main_fs()->get_file_length(_path);
    return result;
}

vfile rmf;
ram_dir *ram_directory[1];
void init_process_userspace_fs(per_process_userspace_fs &target)
{
    utils::context_lock locker(handle_lock);
    for (int i = 0; i < per_process_userspace_fs::ram_files_count; i++)
    {
        target.ram_files[i] = nullptr;
    }

    target.ram_files[0] = new std_zero_file();
    target.ram_files[1] = new std_stdout_file();
    target.ram_files[2] = new std_stderr_file();
    target.ram_files[3] = new std_stdin_file();
}

vfile *process_ramdir::get(const char *msg)
{
    msg += strlen("/proc/");

    pid_t pid = atoi(msg);
    auto process = process::from_pid(pid);
    if (process == nullptr)
    {
        log("ram_dir", LOG_INFO, "invalid pid for path /proc/{} for path+:{}", pid, msg);
        return nullptr;
    }
    for (; *msg != '/'; msg++)
        ;
    if (strncmp(msg, "/fd/", 4) == 0)
    {
        msg += 4;
        int fd = atoi(msg);

        log("ram_dir", LOG_INFO, "getting buffer: {} of process: {} for path: {}", fd, pid, msg);
        return process->get_ufs().ram_files[fd];
    }
    else
    {
        return nullptr;
    }
}

size_t vfile::read(void *buffer, size_t offset, size_t count)
{
    log("ram file", LOG_ERROR, "can't use: {} in file: {} in process: {}", __PRETTY_FUNCTION__, get_npath(), process::current()->get_name());
    return -1;
}
size_t vfile::write(const void *buffer, size_t offset, size_t count)
{
    log("ram file", LOG_ERROR, "can't use: {} in file: {} in process: {}", __PRETTY_FUNCTION__, get_npath(), process::current()->get_name());
    return -1;
}

size_t std_zero_file::read(void *dbuffer, size_t offset, size_t count)
{
    memset(dbuffer, 0, count);
    return offset;
}

size_t std_stdbuf_file::read(void *dbuffer, size_t offset, size_t count)
{
    size_t final_count = count;
    if (size < offset)
    {
        return 0;
    }
    else if (size < final_count + offset)
    {
        final_count = size - offset;
    }
    memcpy(dbuffer, buffer + offset, count);
    return final_count;
}
size_t std_stdbuf_file::write(const void *dbuffer, size_t offset, size_t count)
{
    realocate(offset + count);
    uint8_t *dbuf = (uint8_t *)dbuffer;
    for (size_t i = offset; i < offset + count; i++)
    {
        if (dbuf[i - offset] == '\0')
        {
            buffer[i] = ' ';
        }
        else
        {
            buffer[i] = dbuf[i - offset];
        }
    }
    for (; logging_pos < offset + count; logging_pos++)
    {
        if (buffer[logging_pos] == '\n')
        {
            log("std", LOG_INFO, "{}", range_str(buffer + logging_pos_start, logging_pos - logging_pos_start));
            logging_pos_start = logging_pos + 1;
        }
    }
    return count;
}

void std_stdbuf_file::realocate(size_t new_size)
{
    if (buffer == nullptr)
    {
        buffer = (char *)malloc(new_size);
        memset(buffer, 'f', new_size);
        size = new_size;
        return;
    }
    else
    {
        if (new_size > size)
        {
            buffer = (char *)realloc(buffer, new_size + 1);
        }
        size = new_size;
    }
}

#define MAX_FILE_HANDLE 255
filesystem_file_t *fs_handle_table;

void init_userspace_fs()
{
    fs_handle_table = new filesystem_file_t[MAX_FILE_HANDLE];
    ram_directory[0] = new process_ramdir();
    for (int i = 0; i < MAX_FILE_HANDLE; i++)
    {
        fs_handle_table[i].path = nullptr;
        fs_handle_table[i].state = FS_STATE_FREE;
    }
    add_ram_file(new dev_keyboard_file);
    add_ram_file(new dev_mouse_file);
    add_ram_file(new dev_framebuffer_file);
}

filesystem_file_t *get_if_valid_handle(int fd, bool check_free = true)
{
    if (fd > MAX_FILE_HANDLE)
    {
        log("fs", LOG_WARNING, "process: {} trying to use an invalid file descriptor", process::current()->get_name());
        return nullptr;
    }
    else if (fs_handle_table[fd].state != file_system_file_state::FS_STATE_USED && check_free)
    {
        log("fs", LOG_WARNING, "process: {} trying to use a file descriptor already free", process::current()->get_name());
        return nullptr;
    }
    else if (fs_handle_table[fd].rpid != process::current()->get_parent_pid() && check_free)
    {
        log("fs", LOG_WARNING, "process: {} trying to use a file descriptor used by another process", process::current()->get_name());
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
        log("fs", LOG_WARNING, "process: {} trying to free an invalid file descriptor", process::current()->get_name());
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
        log("fs", LOG_WARNING, "process: {} trying to set used an invalid file descriptor", process::current()->get_name());
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
        log("fs", LOG_WARNING, "process: {} can't read file {}", process::current()->get_name(), fd);
        return 0;
    }
    auto result = file->file->read(buffer, file->cur, count);
    file->cur += result;
    return result;
}

size_t fs_lseek(int fd, size_t offset, int whence)
{

    filesystem_file_t *file = get_if_valid_handle(fd);
    if (file == nullptr)
    {
        log("fs", LOG_WARNING, "process: {} can't seek file with invalid fd {}", process::current()->get_name(), fd);
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
        file->cur = file->file->get_size();
    }
    return file->cur;
}

size_t fs_write(int fd, const void *buffer, size_t count)
{
    filesystem_file_t *file = get_if_valid_handle(fd);
    if (file == nullptr)
    {
        log("fs", LOG_WARNING, "process: {} can't read file with invalid fd {}", process::current()->get_name(), fd);
        return 0;
    }
    auto result = file->file->write(buffer, file->cur, count);
    file->cur += result;
    return result;
}

int init_handle(int fd, const char *path, int mode)
{
    fs_handle_table[fd].rpid = process::current()->get_parent_pid();
    fs_handle_table[fd].path = new char[strlen(path) + 1];
    memcpy(fs_handle_table[fd].path, path, strlen(path) + 1);
    fs_handle_table[fd].mode = mode;
    fs_handle_table[fd].cur = 0;
    fs_handle_table[fd].can_free_handle = false;
    fs_handle_table[fd].file = nullptr;
    return fd;
}

vfile *get_ram_directory_file(const char *path)
{
    ram_dir *target = nullptr;
    for (size_t i = 0; i < sizeof(ram_directory) / sizeof(ram_directory[0]); i++)
    {
        if (strncmp(ram_directory[i]->get_path(), path, strlen(ram_directory[i]->get_path())) == 0)
        {
            target = ram_directory[i];
            break;
        }
    }
    if (target == nullptr)
    {
        return nullptr;
    }
    else
    {
        return target->get(path);
    }
}

int fs_open(const char *path_name, int flags, int mode)
{
    utils::context_lock locker(handle_lock);
    int fd = get_free_handle();
    if (fd == -1)
    {
        log("fs", LOG_WARNING, "process: {} can't get free file descriptor", process::current()->get_name());

        return 0;
    }
    set_used_handle(fd);

    init_handle(fd, path_name, mode);

    // ram file check
    if (is_ram_file(path_name))
    {
        fs_handle_table[fd].file = get_ram_file(path_name);
        return fd;
    }
    // ram directory check
    else if (main_fs_system::the()->main_fs()->get_file_length(path_name) == (uint64_t)-1)
    {
        auto v = get_ram_directory_file(path_name);
        if (v == nullptr)
        {
            log("fs", LOG_WARNING, "trying open file {} but it doesn't exist", process::current()->get_name(), path_name);
            return -1;
        }
        else
        {
            fs_handle_table[fd].file = v;
            return fd;
        }
    }
    // just disk file check
    else
    {
        log("fs", LOG_INFO, "disk-openning: {}", path_name);
        fs_handle_table[fd].file = new disk_file(fs_handle_table[fd].path);
        fs_handle_table[fd].can_free_handle = true;
        return fd;
    }
}

int fs_close(int fd)
{
    auto file = get_if_valid_handle(fd);
    if (file == nullptr)
    {
        log("fs", LOG_ERROR, "process {} invalid fd for closing file", process::current()->get_name());

        return 0;
    }
    utils::context_lock locker(handle_lock);

    set_free_handle(fd);
    delete[] fs_handle_table[fd].path;

    if (fs_handle_table[fd].can_free_handle)
    {
        free(fs_handle_table[fd].file);
    }
    return 1;
}

bool is_ram_file(const char *path)
{
    for (size_t i = 0; i < ram_file_list.size(); i++)
    {

        if (strcmp(ram_file_list[i]->get_npath(), path) == 0)
        {
            return true;
        }
    }
    for (int i = 0; i < per_process_userspace_fs::ram_files_count; i++)
    {
        if (process::current()->get_ufs().ram_files[i] == nullptr)
        {
            continue;
        }
        if (strcmp(process::current()->get_ufs().ram_files[i]->get_npath(), path) == 0)
        {
            return true;
        }
    }

    return false;
}

vfile *get_ram_file(const char *path)
{

    for (size_t i = 0; i < ram_file_list.size(); i++)
    {

        if (strcmp(ram_file_list[i]->get_npath(), path) == 0)
        {
            return ram_file_list[i];
        }
    }
    for (int i = 0; i < per_process_userspace_fs::ram_files_count; i++)
    {
        if (process::current()->get_ufs().ram_files[i] == nullptr)
        {
            continue;
        }
        if (strcmp(process::current()->get_ufs().ram_files[i]->get_npath(), path) == 0)
        {
            return process::current()->get_ufs().ram_files[i];
        }
    }
    return nullptr;
}
