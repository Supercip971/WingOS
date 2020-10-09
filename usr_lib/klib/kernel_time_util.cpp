#include <klib/kernel_time_util.h>
#include <klib/process_message.h>
namespace sys {

    time_t get_time_total_sec(){
        time_service_request tsr = {0};
        tsr.get = true;
        tsr.index = sys::get_current_stime;
        tsr.value = -1;
        sys::process_message msg = sys::process_message("time_service", (uint64_t)&tsr, sizeof (tsr));
        uint64_t result = msg.read();
        return result;
    }
}
