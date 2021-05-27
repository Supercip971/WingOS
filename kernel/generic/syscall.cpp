#include <device/local_data.h>
#include <filesystem/file_system.h>
#include <logging.h>
#include <module/module_calls.h>
#include <module_calls.h>
#include <proc/msg_system.h>
#include <proc/process.h>
#include <programm_launcher.h>
#include <syscall.h>
#include <utils/sys/proc_info_flag.h>
#include <utils/sys/programm_exec_info.h>

utils::lock_type msg_lock;

uint64_t sys$null(const char *arg1)
{
    log("syscall", LOG_ERROR, "null syscall called");

    return 32;
}

int sys$set_modules_calls(module_calls_list *target)
{
    if (!process::current()->is_module())
    {
        log("syscall", LOG_ERROR, "only a module can access the {} syscall", __PRETTY_FUNCTION__);
        return 0;
    }
    set_modules_calls(target);
    return 1;
}

void *sys$alloc(uint64_t count, uint8_t flag)
{
    auto res = (pmm_alloc_zero(count));
    if (flag & SYS_ALLOC_SHARED || process::current()->is_module())
    {
        log("syscall", LOG_INFO, "(shared) allocating {}", (void *)get_mem_addr(res));
        return (void *)get_mem_addr(res);
    }
    else
    {
        uintptr_t addr = process::current()->allocate_virtual_addr(count) * PAGE_SIZE;
        for (uint64_t i = 0; i < count; i++)
        {
            map_page((uintptr_t)res + i * PAGE_SIZE, get_usr_addr(addr) + i * PAGE_SIZE, true, true);
        }
        update_paging();
        log("syscall", LOG_INFO, "(local) allocating {}", (void *)get_usr_addr(addr));
        return (void *)get_usr_addr(addr);
    }
}

int sys$free(uintptr_t target, uint64_t count)
{
    if (target >= MEM_ADDR)
    {
        log("syscall", LOG_INFO, "(local) freeing {}", target);

        pmm_free((void *)get_physical_addr(target), count);
    }
    else
    {
        log("syscall", LOG_INFO, "(shared) freeing {}", target);

        process::current()->free_virtual_addr(get_rusr_addr(target) / PAGE_SIZE, count);

        pmm_free((void *)get_physical_addr(target), count);
        unmap_page(process::current()->get_arch_info()->page_directory, target);
    }
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

pid_t sys$getpid()
{
    return process::current()->get_pid();
}

pid_t sys$exec(programm_exec_info *info)
{
    return launch_programm_usr(info);
}

size_t sys$exit(int code)
{
    if (code != 0)
    {
        log("proc", LOG_ERROR, "process exiting with code: {}", code);
    }
    kill_current(code);

    return 0;
}

utils::lock_type temporary_lock;
int sys$ipc_server_exist(const char *path)
{
    utils::context_lock lock(temporary_lock);
    return service_exist(path);
}

int sys$create_server(const char *path)
{
    return create_msg_system(path, process::current()->get_parent_pid());
}

uint32_t sys$connect_to_server(const char *path)
{
    return connect(path, process::current()->get_parent_pid());
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

size_t sys$get_process_info(pid_t pid, int flag, void *arg1, void *arg2)
{
    if (flag == PROC_INFO_STATUS)
    {
        get_process_status(pid, (int *)arg1, (int *)arg2);
        return 1;
    }
    else
    {
        log("syscall", LOG_ERROR, "{} invalid flag: {}", __PRETTY_FUNCTION__, flag);
        return 0;
    }
}

static void *syscalls[] = {
    (void *)sys$null,
    (void *)sys$set_modules_calls,
    (void *)sys$null,
    (void *)sys$null,
    (void *)sys$null,
    (void *)sys$null,
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
    (void *)sys$get_process_info};

uint64_t syscalls_length = sizeof(syscalls) / sizeof(void *);

void init_syscall()
{
    log("syscall", LOG_DEBUG, "loading syscall");
    // for later
}

uint64_t syscall(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, cpu_registers *stackframe)
{
    if (syscall_id > syscalls_length)
    {
        log("syscall", LOG_ERROR, "called invalid syscall: {}", syscall_id);
        return 0;
    }
    else
    {
        //    log("syscall", LOG_INFO, "syscall: {}, from: {}", syscall_id, process::current()->get_name());
        uint64_t (*func)(uint64_t, ...) = reinterpret_cast<uint64_t (*)(uint64_t, ...)>(syscalls[syscall_id]);

        uint64_t res = func(arg1, arg2, arg3, arg4, arg5);
        return res;
    }
}
