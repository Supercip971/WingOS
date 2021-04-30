#include "module_calls.h"
#ifdef MODULE
#include <kern/syscall.h>
module_calls_list calls_list;
static inline uintptr_t sys$set_module_call()
{
    return sys::syscall((uintptr_t)syscall_codes::SET_MODULES_CALLS, (uintptr_t)&calls_list, 0, 0, 0, 0);
}
void call_init()
{
    sys$set_module_call();
}

void echo_out(const char *data)
{
    calls_list.echo_out(data);
}

size_t get_kern_version()
{
    return calls_list.get_kernel_version();
}

size_t io_function(size_t addr, size_t write_val, io_func_exec flags)
{
    return calls_list.io_func(addr, write_val, flags);
}

int set_device_driver(uint32_t id, general_device *as)
{
    return calls_list.set_device_driver(id, as);
}
const general_device *get_device_driver(uint32_t id)
{
    return calls_list.get_device_driver(id);
}

int set_module_name(const char *name)
{
    return calls_list.set_module_name(name);
}
int add_irq_handler(size_t irq, mfunc<void, uint32_t> func)
{
    return calls_list.add_irq_handler(irq, func);
}
#endif
