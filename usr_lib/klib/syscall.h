#pragma once
#include <stdint.h>
#include <klib/process_message.h>
namespace sys {

    enum syscall_codes
    {
        NULL_SYSCALL = 0,                   // or debug syscall
        SEND_SERVICE_SYSCALL = 1,           // send a message to a service
        READ_SERVICE_SYSCALL = 2,           // read all message that you have
        GET_RESPONSE_SERVICE_SYSCALL = 3,   // if the message sended has been responded
        GET_PROCESS_GLOBAL_DATA = 4         // get process global data, if arg1 (target) is nullptr, return self global data, else return a process global data return -1 if there is an error
    };


    static inline uint64_t syscall(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5){
        uint64_t kernel_return = 0;
        asm volatile(
                     "int 0x7f"
                     "":"=a"(kernel_return) : "0"(syscall_id), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5) : "memory"); // for debugging
        return kernel_return;
    }
    raw_process_message* sys$send_message(uint64_t data_addr, uint64_t data_length, const char* target);
    raw_process_message* sys$read_message();
    uint64_t sys$message_response(raw_process_message* identifier);
    uint64_t sys$get_process_global_data(uint64_t offset, const char* target);
    void* sys$get_current_process_global_data(uint64_t offset, uint64_t length);


}
