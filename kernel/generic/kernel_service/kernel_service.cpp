#include <kernel_service/filesystem_service.h>
#include <kernel_service/graphic_buffer_service.h>
#include <kernel_service/kernel_buffer_service.h>
#include <kernel_service/kernel_process_service.h>
#include <kernel_service/kernel_service.h>
#include <kernel_service/print_service.h>
#include <kernel_service/ps2_device_service.h>
#include <kernel_service/time_service.h>
#include <logging.h>
#include <process.h>

void add_kernel_service(func entry, const char *service_name)
{
    //   log("kernel service", LOG_INFO) << "launching service : " << service_name;

    init_process((func)entry, true, service_name, false, AUTO_SELECT_CPU);
}
void load_kernel_service()
{
    log("kernel service", LOG_DEBUG) << "loading kernel service";

    add_kernel_service(kernel_process_service, "kernel_process_service");
    //add_kernel_service(print_service, "console_out");
    add_kernel_service(graphic_buffer_service, "graphic_buffer_service");
    //add_kernel_service(time_service, "time_service");
    //  add_kernel_service(ps2_device_service, "ps2_device_service");
    // add_kernel_service(file_system_service, "file_system_service");
}
