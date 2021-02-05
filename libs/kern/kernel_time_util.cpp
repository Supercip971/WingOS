#include <kern/kernel_time_util.h>
#include <kern/process_message.h>
namespace sys
{

    time_t get_time_total_sec()
    {
        time_service_request tsr = {0};
        tsr.get = true;
        tsr.index = sys::get_current_stime;
        tsr.value = -1;
        sys::service_message msg = sys::service_message("time_service", (uint64_t)&tsr, sizeof(tsr));
        uint64_t result = msg.read();
        return result;
    }

} // namespace sys
