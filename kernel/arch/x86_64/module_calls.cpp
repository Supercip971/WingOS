#include "module_calls.h"
#include <arch.h>
#include <interrupt.h>
#include <logging.h>
void M_echo_out(const char *data)
{
    log("module", LOG_INFO, data);
}

size_t M_get_kernel_version()
{
    return 1;
}

size_t M_io_func(size_t addr, size_t write, io_func_exec flags)
{

    if (flags.use_memory)
    {
        log("module", LOG_ERROR, "can't use memory io for the moment");
        return 0;
    }
    if (flags.bit_size == IO_BIT_SIZE_64)
    {
        log("module", LOG_ERROR, "can't use memory bitsize 64 for the moment");
        return 0;
    }
    if (flags.write)
    {
        if (flags.bit_size == IO_BIT_SIZE_8)
        {
            outb(addr, write);
        }
        if (flags.bit_size == IO_BIT_SIZE_16)
        {
            outl(addr, write);
        }
        if (flags.bit_size == IO_BIT_SIZE_32)
        {
            outw(addr, write);
        }
        return 1;
    }
    else
    {

        if (flags.bit_size == IO_BIT_SIZE_8)
        {
            return inb(addr);
        }
        if (flags.bit_size == IO_BIT_SIZE_16)
        {
            return inl(addr);
        }
        if (flags.bit_size == IO_BIT_SIZE_32)
        {
            return inw(addr);
        }
        return 1;
    }
}
int M_set_device_driver(uint32_t id, general_device *target)
{

    add_device(target);
    return 1;
}
const general_device *M_get_device_driver(uint32_t id)
{
    return get_device(id);
}
bool M_add_irq_handler(size_t irq, mfunc<void, uint32_t> func)
{
    add_irq_handler(func, irq);
    return true;
}
void set_modules_calls(module_calls_list *list)
{
    list->echo_out = M_echo_out;
    list->null_func = nullptr;
    list->get_kernel_version = M_get_kernel_version;
    list->io_func = M_io_func;
    list->set_device_driver = M_set_device_driver;
    list->get_device_driver = M_get_device_driver;
    list->add_irq_handler = M_add_irq_handler;
}
