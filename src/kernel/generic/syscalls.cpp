#include "syscalls.hpp"
#include "libcore/fmt/log.hpp"
core::Result<size_t> syscall_handle(SyscallInterface syscall)
{
    switch (syscall.id)
    {
        case SYSCALL_DEBUG_LOG_ID:
        {
            auto debug = syscall_debug_decode(syscall);
            log::log$("DEBUG: {}", debug.message);
            return core::Result<size_t>::success(0);
        }
        default:
            return {"Unknown syscall ID"};
    }
}
