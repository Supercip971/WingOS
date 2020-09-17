#include <loggging.h>
#include <syscall.h>
typedef uint64_t (*syscall_functions)(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

uint64_t sys$null(uint64_t arg1, uint64_t arg2)
{
    log("syscall", LOG_INFO) << "receive null syscall with arg1 : " << arg1 << "arg 2 : " << arg2;
    return 32;
}
static void *syscalls[] = {
    [0] = (void *)sys$null};
void init_syscall()
{
    log("syscall", LOG_DEBUG) << "loading syscall";
}
uint64_t syscall(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
    if (syscall_id != 0)
    {
        log("syscall", LOG_ERROR) << "called null syscall";
        return -1;
    }
    else
    {
        uint64_t (*func)(uint64_t, ...) = reinterpret_cast<uint64_t (*)(uint64_t, ...)>(syscalls[syscall_id]);
        return func(arg1, arg2, arg3, arg4, arg5);
    }
}
