#include <klib/kernel_util.h>
#include <klib/process_message.h>
namespace sys {

    int write_console(const char* raw_data, int length){
        if(length < 0){
            return -2;
        }
        if(raw_data == nullptr){
            return -1;
        }
        sys::process_message("console_out", (uint64_t)raw_data, length);
        return 1;
    }
}
