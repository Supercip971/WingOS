#pragma once
#include <kern/process_message.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <utils/raw_msg_system.h>
#include <utils/programm_exec_info.h>
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

    static inline uintptr_t sys$get_process_global_data(uint64_t offset, const char *target)
    {
        return syscall((uintptr_t)syscall_codes::GET_PROCESS_GLOBAL_DATA, (uint64_t)target, offset, 0, 0, 0);
    }

    static inline void *sys$get_current_process_global_data(uint64_t offset, uint64_t length)
    {
        return (void *)syscall((uintptr_t)syscall_codes::GET_PROCESS_GLOBAL_DATA, 0, offset, length, 0, 0);
    }

    static inline void *sys$alloc(uintptr_t count, uint8_t flag)
    {
        return (void *)syscall((uintptr_t)syscall_codes::MEMORY_ALLOC, count, flag, 0, 0, 0);
    }
    static inline int sys$free(uintptr_t target, uint64_t count)
    {
        return syscall((uintptr_t)syscall_codes::MEMORY_FREE, target, count, 0, 0, 0);
    }
    static inline size_t sys$read(int fd, void *buffer, size_t count)
    {
        return syscall((uintptr_t)syscall_codes::FILE_READ, fd, (uint64_t)buffer, count, 0, 0);
    }
    static inline size_t sys$write(int fd, const void *buffer, size_t count)
    {
        return syscall((uintptr_t)syscall_codes::FILE_WRITE, fd, (uint64_t)buffer, count, 0, 0);
    }
    static inline int sys$open(const char *path_name, int flags, int mode)
    {
        return syscall((uintptr_t)syscall_codes::FILE_OPEN, (uint64_t)path_name, flags, mode, 0, 0);
    }
    static inline int sys$close(int fd)
    {
        return syscall((uintptr_t)syscall_codes::FILE_CLOSE, fd, 0, 0, 0, 0);
    }
    static inline size_t sys$lseek(int fd, size_t offset, int whence)
    {
        return syscall((uintptr_t)syscall_codes::FILE_SEEK, fd, offset, whence, 0, 0);
    }
    static inline int sys$nano_sleep(const timespec *request, timespec *remaning)
    {
        return syscall((uintptr_t)syscall_codes::NANO_SLEEP, (uintptr_t)request, (uintptr_t)remaning, 0, 0, 0);
    }
    static inline int sys$getpid()
    {
        return syscall((uintptr_t)syscall_codes::GET_PID, 0, 0, 0, 0, 0);
    }
    static inline int sys$exec(programm_exec_info *execution_info)
    {
        return syscall((uintptr_t)syscall_codes::EXEC, (uintptr_t)execution_info, 0, 0, 0, 0);
    }
    static inline int sys$exit(int code)
    {
        return syscall((uintptr_t)syscall_codes::EXIT, (uintptr_t)code, 0, 0, 0, 0);
    }
    static inline int sys$ipc_server_exist(const char* path){
        return syscall((uintptr_t)syscall_codes::IPC_SERV_EXIST, (uintptr_t)path, 0,0,0,0);
    }
    
    static inline int sys$create_server(const char* path){
        return syscall((uintptr_t)syscall_codes::CREATE_SERVER, (uintptr_t)path, 0,0,0,0);
    }
    
    static inline uint32_t sys$connect_to_server(const char* path){
        return syscall((uintptr_t)syscall_codes::CONNECT_SERVER, (uintptr_t)path, 0,0,0,0);
    }
    
    static inline uint32_t sys$accept_connection(int server_id){
        return syscall((uintptr_t)syscall_codes::ACCEPT_CONNECTION, (uintptr_t)server_id, 0,0,0,0);
    }

    
    static inline int sys$is_connection_accepted(uint32_t id){
        return syscall((uintptr_t)syscall_codes::IS_CONNECTION_ACCEPTED, (uintptr_t)id, 0,0,0,0);
    }

    
    static inline int sys$deconnect(uint32_t id){
        return syscall((uintptr_t)syscall_codes::DECONNECT, (uintptr_t)id, 0,0,0,0);
    }

    
    static inline size_t sys$send(uint32_t id, const raw_msg_request *request, int flags){
        return syscall((uintptr_t)syscall_codes::SEND, (uintptr_t)id, (uintptr_t)request,(uintptr_t)flags,0,0);
    }
    
    static inline size_t sys$receive(uint32_t id, raw_msg_request *request, int flags){
        return syscall((uintptr_t)syscall_codes::RECEIVE, (uintptr_t)id, (uintptr_t)request,(uintptr_t)flags,0,0);
    }

} // namespace sys
