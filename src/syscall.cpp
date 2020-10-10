#include <arch/lock.h>
#include <arch/process.h>
#include <loggging.h>
#include <syscall.h>
typedef uint64_t (*syscall_functions)(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

uint64_t sys$null(uint64_t arg1, uint64_t arg2)
{
    //  log("syscall", LOG_INFO) << "receive null syscall with arg1 : " << arg1 << "arg 2 : " << arg2;
    return 32;
}
process_message *sys$send_message(uint64_t data_addr, uint64_t data_length, const char *to_process)
{
    return send_message(data_addr, data_length, to_process);
}
process_message *sys$read_message()
{
    return read_message();
}
uint64_t sys$message_response(process_message *identifier)
{
    return message_response(identifier);
}
uint64_t sys$get_process_global_data(const char* target, uint64_t offset, uint64_t length){
    if(target == nullptr){
        return (uint64_t)get_current_process_global_data(offset, length);
    }else{
        return (uint64_t)get_process_global_data_copy(offset, target);
    }
}
static void *syscalls[] = {
    [0] = (void *)sys$null,
    [SEND_SERVICE_SYSCALL] = (void *)sys$send_message,
    [READ_SERVICE_SYSCALL] = (void *)sys$read_message,
    [GET_RESPONSE_SERVICE_SYSCALL] = (void *)sys$message_response,
    [GET_PROCESS_GLOBAL_DATA] = (void*)sys$get_process_global_data
};
uint64_t syscalls_length = 5;
void init_syscall()
{
    log("syscall", LOG_DEBUG) << "loading syscall";
}
uint64_t syscall(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
    if (syscall_id > syscalls_length)
    {
        log("syscall", LOG_ERROR) << "called invalid syscall" << syscall_id;
        return 0;
    }
    else
    {

        uint64_t (*func)(uint64_t, ...) = reinterpret_cast<uint64_t (*)(uint64_t, ...)>(syscalls[syscall_id]);

        uint64_t res = func(arg1, arg2, arg3, arg4, arg5);
        return res;
    }
}
