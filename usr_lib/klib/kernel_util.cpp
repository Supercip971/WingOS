#include <klib/kernel_util.h>
#include <klib/process_message.h>
#include <string.h>
namespace sys {

    int write_console(const char* raw_data, int length){
        if(length < 0){
            return -2;
        }
        if(raw_data == nullptr){
            return -1;
        }
        sys::process_message("console_out", (uint64_t)raw_data, length).read();
        return 1;
    }

    uint64_t get_process_pid(const char* process_name){
        uint64_t result = sys::process_message("kernel_process_service", (uint64_t)process_name, strlen(process_name)+1 ).read();
        return result;
    }
}
