
#include <arch/process.h>
#include <device/local_data.h>
#include <logging.h>
#include <syscall.h>
lock_type lck_syscall = {0};
typedef uint64_t (*syscall_functions)(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
InterruptStackFrame *stakframe_testing;
uint64_t sys$null(uint64_t arg1, uint64_t arg2)
{
    //  log("syscall", LOG_INFO) << "receive null syscall with arg1 : " << arg1 << "arg 2 : " << arg2;
    return 32;
}
process_message *sys$send_message(uintptr_t data_addr, uint64_t data_length, const char *to_process)
{
    auto res = send_message(data_addr, data_length, to_process);
    if (res == nullptr)
    {
        dumpregister(stakframe_testing);
        log("syscall", LOG_ERROR) << "in process" << get_current_cpu()->current_process->process_name;
        dump_backtrace("back", get_current_cpu()->current_process->rip_backtrace);
    };
    return res;
}
process_message *sys$read_message()
{
    return read_message();
}
uint64_t sys$message_response(process_message *identifier)
{
    return message_response(identifier);
}
uint64_t sys$get_process_global_data(const char *target, uint64_t offset, uint64_t length)
{
    if (target == nullptr)
    {
        return (uintptr_t)get_current_process_global_data(offset, length);
    }
    else
    {
        return (uintptr_t)get_process_global_data_copy(offset, target);
    }
}
static void *syscalls[] = {
    (void *)sys$null,
    (void *)sys$send_message,
    (void *)sys$read_message,
    (void *)sys$message_response,
    (void *)sys$get_process_global_data};
uint64_t syscalls_length = 5;
void init_syscall()
{
    log("syscall", LOG_DEBUG) << "loading syscall";
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
        lock(&lck_syscall);
        stakframe_testing = stackframe;
        //  log("syscall", LOG_INFO) << "syscall " << syscall_id << "from : " << get_current_cpu()->current_process->process_name;
        uint64_t (*func)(uint64_t, ...) = reinterpret_cast<uint64_t (*)(uint64_t, ...)>(syscalls[syscall_id]);

        uint64_t res = func(arg1, arg2, arg3, arg4, arg5);
        unlock(&lck_syscall);
        return res;
    }
}
