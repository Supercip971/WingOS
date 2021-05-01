#ifndef MODULE_CALLS_H
#define MODULE_CALLS_H
#include <module/device_generic_driver.h>
#include <stddef.h>
#include <stdint.h>
template <typename T, typename... argument>
using mfunc = T (*)(argument...);
#define IO_BIT_SIZE_8 0b0
#define IO_BIT_SIZE_16 0b01
#define IO_BIT_SIZE_32 0b10
#define IO_BIT_SIZE_64 0b11

struct io_func_exec
{
    bool write : 1;
    uint8_t bit_size : 2;
    uint8_t return_result : 1;
    uint8_t use_memory : 1;
};

struct module_calls_list
{
    mfunc<void> null_func;
    mfunc<void, const char *> echo_out;
    mfunc<size_t> get_kernel_version;
    mfunc<size_t, size_t, size_t, io_func_exec> io_func;
    mfunc<int, uint32_t, general_device *> set_device_driver;
    mfunc<const general_device *, uint32_t> get_device_driver;
    mfunc<int, const char *> set_module_name;
    mfunc<bool, size_t, mfunc<void, uint32_t>> add_irq_handler;
} __attribute__((packed));
#ifdef MODULE
void call_init();

void echo_out(const char *data);

size_t get_kern_version();

size_t io_function(size_t addr, size_t write_val, io_func_exec flags);

int set_device_driver(uint32_t id, general_device *as);
const general_device *get_device_driver(uint32_t id);

int set_module_name(const char *name);
int add_irq_handler(size_t irq, mfunc<void, uint32_t> func);

// just equivalent of cli/sti
static inline void enter_low_level_context()
{
    asm volatile("cli");
}
static inline void exit_low_level_context()
{
    asm volatile("sti");
}
inline void outb(uint16_t port, uint8_t value)
{
    asm volatile("out  dx, al" ::"a"(value), "d"(port));
}

inline void outw(uint16_t port, uint16_t value)
{
    asm volatile("out  dx, ax" ::"a"(value), "d"(port));
}

inline void outl(uint16_t port, uint32_t value)
{
    asm volatile("out  dx, eax" ::"a"(value), "d"(port));
}

inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("in al, dx"
                 : "=a"(ret)
                 : "d"(port));
    return ret;
}

inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile("in ax, dx"
                 : "=a"(ret)
                 : "d"(port));
    return ret;
}

inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile("in eax, dx"
                 : "=a"(ret)
                 : "d"(port));
    return ret;
}

#endif
#endif // MODULE_CALLS_H
