#include <device/local_data.h>
#include <filesystem/file_system.h>
#include <logging.h>
#include <msg_system.h>
#include <process.h>
#include <programm_launcher.h>
#include <syscall.h>
#include <utils/programm_exec_info.h>

utils::lock_type msg_lock;

uint64_t sys$null(const char *arg1)
{
    log(process::current()->get_name(), LOG_INFO) << arg1;

    return 32;
}

process_message *sys$send_message(uintptr_t data_addr, uint64_t data_length, const char *to_process)
{
    msg_lock.lock();
    auto res = send_message(data_addr, data_length, to_process);

    msg_lock.unlock();
    return res;
}

process_message *sys$read_message()
{
    msg_lock.lock();
    auto v = read_message();

    msg_lock.unlock();
    return v;
}

uint64_t sys$message_response(process_message *identifier)
{
    msg_lock.lock();
    auto v = message_response(identifier);

    msg_lock.unlock();
    return v;
}

uint64_t sys$get_process_global_data(const char *target, uint64_t offset, uint64_t length)
{

    msg_lock.lock();
    uintptr_t v = 0;
    if (target == nullptr)
    {
        v = (uintptr_t)get_current_process_global_data(offset, length);
    }
    else
    {
        v = (uintptr_t)get_process_global_data_copy(offset, target);
    }

    msg_lock.unlock();
    return v;
}

process_message *sys$send_message_pid(uintptr_t data_addr, uint64_t data_length, uint64_t to_process)
{
    msg_lock.lock();
    auto res = send_message_pid(data_addr, data_length, to_process);

    msg_lock.unlock();
    return res;
}

void *sys$alloc(uint64_t count, uint8_t flag)
{
    auto res = (pmm_alloc(count));
    if (flag & SYS_ALLOC_SHARED)
    {

        return (void *)get_mem_addr(res);
    }
    else
    {

        for (uint64_t i = 0; i < count; i++)
        {
            map_page((uintptr_t)res + i * PAGE_SIZE, get_usr_addr(res) + i * PAGE_SIZE, PAGE_TABLE_FLAGS);
        }
        update_paging();
        return (void *)get_usr_addr(res);
    }
}

int sys$free(uintptr_t target, uint64_t count)
{
    pmm_free((void *)get_rusr_addr(target), count);
    return 1;
}

size_t sys$read(int fd, void *buffer, size_t count)
{
    return fs_read(fd, buffer, count);
}

size_t sys$write(int fd, const void *buffer, size_t count)
{
    return fs_write(fd, buffer, count);
}

int sys$open(const char *path_name, int flags, int mode)
{
    return fs_open(path_name, flags, mode);
}

int sys$close(int fd)
{
    return fs_close(fd);
}

size_t sys$lseek(int fd, size_t offset, int whence)
{
    return fs_lseek(fd, offset, whence);
}

int sys$nano_sleep(const timespec *request, timespec *remaning)
{
    uint64_t total_time = request->tv_nsec / 1000000 + request->tv_sec * 1000;
    sleep(total_time);
    return 0;
}

size_t sys$getpid()
{
    return process::current()->get_pid();
}

size_t sys$exec(programm_exec_info *info)
{
    return launch_programm_usr(info);
}

size_t sys$exit(int code)
{
    if (code != 0)
    {
        log("proc", LOG_ERROR, "process exiting with code: {}", code);
    }
    kill_current();

    return 0;
}

int sys$ipc_server_exist(const char *path)
{
    return service_exist(path);
}
int sys$create_server(const char *path)
{
    return create_msg_system(path, process::current()->get_pid());
}
uint32_t sys$connect_to_server(const char *path)
{
    return connect(path, process::current()->get_pid());
}
uint32_t sys$accept_connection(int server_id)
{
    return accept_connection(server_id);
}

int sys$is_connection_accepted(uint32_t id)
{
    return connection_accepted(id);
}

int sys$deconnect(uint32_t id)
{
    return deconnect(id);
}

size_t sys$send(uint32_t id, const raw_msg_request *request, int flags)
{
    return send(id, request, flags);
}
size_t sys$receive(uint32_t id, raw_msg_request *request, int flags)
{
    return receive(id, request, flags);
}
static void *syscalls[] = {
    (void *)sys$null,
    (void *)sys$send_message,
    (void *)sys$read_message,
    (void *)sys$message_response,
    (void *)sys$get_process_global_data,
    (void *)sys$send_message_pid,
    (void *)sys$alloc,
    (void *)sys$free,
    (void *)sys$open,
    (void *)sys$close,
    (void *)sys$read,
    (void *)sys$write,
    (void *)sys$lseek,
    (void *)sys$nano_sleep,
    (void *)sys$getpid,
    (void *)sys$exec,
    (void *)sys$exit,
    (void *)sys$ipc_server_exist,
    (void *)sys$create_server,
    (void *)sys$connect_to_server,
    (void *)sys$accept_connection,
    (void *)sys$is_connection_accepted,
    (void *)sys$deconnect,
    (void *)sys$send,
    (void *)sys$receive,
};

uint64_t syscalls_length = sizeof(syscalls) / sizeof(void *);

void init_syscall()
{
    log("syscall", LOG_DEBUG) << "loading syscall";
    // for later
}

uint64_t syscall(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, InterruptStackFrame *stackframe)
{
    if (syscall_id > syscalls_length)
    {
        log("syscall", LOG_ERROR) << "called invalid syscall" << syscall_id;
        return 0;
    }
    else
    {
        //  log("syscall", LOG_INFO) << "syscall " << syscall_id << "from : " << get_current_cpu()->current_process->process_name;
        uint64_t (*func)(uint64_t, ...) = reinterpret_cast<uint64_t (*)(uint64_t, ...)>(syscalls[syscall_id]);

        uint64_t res = func(arg1, arg2, arg3, arg4, arg5);
        return res;
    }
}
