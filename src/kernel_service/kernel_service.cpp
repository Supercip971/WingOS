#include <arch/process.h>
#include <kernel_service/graphic_buffer_service.h>
#include <kernel_service/kernel_service.h>
#include <kernel_service/memory_service.h>
#include <kernel_service/print_service.h>
#include <kernel_service/ps2_device_service.h>
#include <kernel_service/time_service.h>
#include <loggging.h>

void add_kernel_service(func entry, const char *service_name)
{
    log("kernel service", LOG_INFO) << "launching service : " << service_name;

    process *service = init_process((func)entry, true, service_name, false, -2);
    while (service->pid == 0)
    {
        service = init_process((func)entry, true, service_name, false, -2);
    }
}
void load_kernel_service()
{
    log("kernel service", LOG_DEBUG) << "loading kernel service";
    add_kernel_service(print_service, "console_out");
    add_kernel_service(memory_service, "memory_service");
    add_kernel_service(graphic_buffer_service, "graphic_buffer_service");
    add_kernel_service(time_service, "time_service");
    add_kernel_service(ps2_device_service, "ps2_device_service");
}
