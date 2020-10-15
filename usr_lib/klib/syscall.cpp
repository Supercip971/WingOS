#include <klib/syscall.h>


namespace sys {

    raw_process_message* sys$send_message(uint64_t data_addr, uint64_t data_length, const char* target){
       return (raw_process_message*) syscall(SEND_SERVICE_SYSCALL, data_addr, data_length, (uint64_t)target, 0,0);
    }

    raw_process_message* sys$read_message(){
        return (raw_process_message*) syscall(READ_SERVICE_SYSCALL, 0, 0, 0, 0,0);
    }

    uint64_t sys$message_response(raw_process_message* identifier){
        return syscall(GET_RESPONSE_SERVICE_SYSCALL, (uint64_t)identifier, 0, 0, 0,0);
    }

    uint64_t sys$get_process_global_data(uint64_t offset, const char* target){
        return syscall(GET_PROCESS_GLOBAL_DATA, (uint64_t)target, offset, 0, 0, 0);
    }

    void* sys$get_current_process_global_data(uint64_t offset, uint64_t length){
        return (void*)syscall(GET_PROCESS_GLOBAL_DATA, 0, offset, length, 0, 0);
    }
}
