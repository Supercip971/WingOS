#pragma once
#include <kern/process_message.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <utils/syscall_codes.h>
namespace sys
{

    __attribute__((optimize("O0"), always_inline)) inline uint64_t syscall(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5)
    {
        uint64_t kernel_return = 0;
        asm volatile(
            "int 0x7f"
            ""
            : "=a"(kernel_return)
            : "0"(syscall_id), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
            : "memory"); // for debugging
        return kernel_return;
    };
    static inline raw_process_message *sys$send_message(uint64_t data_addr, uint64_t data_length, const char *target)
    {
        return (raw_process_message *)syscall((uint64_t)syscall_codes::SEND_SERVICE_SYSCALL, data_addr, data_length, (uint64_t)target, 0, 0);
    }

    static inline raw_process_message *sys$read_message()
    {
        return (raw_process_message *)syscall((uint64_t)syscall_codes::READ_SERVICE_SYSCALL, 0, 0, 0, 0, 0);
    }

    static inline uint64_t sys$message_response(raw_process_message *identifier)
    {
        return syscall((uint64_t)syscall_codes::GET_RESPONSE_SERVICE_SYSCALL, (uint64_t)identifier, 0, 0, 0, 0);
    }

    static inline uint64_t sys$get_process_global_data(uint64_t offset, const char *target)
    {
        return syscall((uint64_t)syscall_codes::GET_PROCESS_GLOBAL_DATA, (uint64_t)target, offset, 0, 0, 0);
    }

    static inline void *sys$get_current_process_global_data(uint64_t offset, uint64_t length)
    {
        return (void *)syscall((uint64_t)syscall_codes::GET_PROCESS_GLOBAL_DATA, 0, offset, length, 0, 0);
    }
    static inline raw_process_message *sys$send_message_pid(uint64_t data_addr, uint64_t data_length, uint64_t pid)
    {
        return (raw_process_message *)syscall((uint64_t)syscall_codes::SEND_PROCESS_SYSCALL_PID, data_addr, data_length, pid, 0, 0);
    }
    static inline void *sys$alloc(uint64_t count)
    {
        return (void *)syscall((uint64_t)syscall_codes::MEMORY_ALLOC, count, 0, 0, 0, 0);
    }
    static inline int sys$free(uintptr_t target, uint64_t count)
    {
        return syscall((uint64_t)syscall_codes::MEMORY_FREE, target, count, 0, 0, 0);
    }
    static inline size_t sys$read(int fd, void *buffer, size_t count)
    {
        return syscall((uint64_t)syscall_codes::FILE_READ, fd, (uint64_t)buffer, count, 0, 0);
    }
    static inline size_t sys$write(int fd, const void *buffer, size_t count)
    {
        return syscall((uint64_t)syscall_codes::FILE_WRITE, fd, (uint64_t)buffer, count, 0, 0);
    }
    static inline int sys$open(const char *path_name, int flags, int mode)
    {
        return syscall((uint64_t)syscall_codes::FILE_OPEN, (uint64_t)path_name, flags, mode, 0, 0);
    }
    static inline int sys$close(int fd)
    {
        return syscall((uint64_t)syscall_codes::FILE_CLOSE, fd, 0, 0, 0, 0);
    }
    static inline size_t sys$lseek(int fd, size_t offset, int whence)
    {
        return syscall((uint64_t)syscall_codes::FILE_SEEK, fd, offset, whence, 0, 0);
    }
    static inline int sys$nano_sleep(const timespec *request, timespec *remaning)
    {
        return syscall((uintptr_t)syscall_codes::NANO_SLEEP, (uintptr_t)request, (uintptr_t)remaning, 0, 0, 0);
    }
    static inline int sys$getpid()
    {
        return syscall((uintptr_t)syscall_codes::GET_PID, 0, 0, 0, 0, 0);
    }

} // namespace sys
