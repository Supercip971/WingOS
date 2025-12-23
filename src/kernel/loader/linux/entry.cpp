
#include <arch/x86/com.hpp>
#include <kernel/generic/kernel.hpp>
#include <kernel/loader/limine/limine.hpp>
#include <libcore/fmt/fmt.hpp>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
 #include <sys/syscall.h>      /* Definition of SYS_* constants */

#include "libcore/fmt/log.hpp"
#include "libcore/io/writer.hpp"
#include "libcore/str.hpp"
#include "mcx/mcx.hpp"
void _start(void);

extern "C" long syscall(long number, ...);

static void done(void)
{
    for (;;)
    {
        __asm__("hlt");
    }
}
extern "C" uintptr_t kernel_physical_base()
{
    return 0;
}
extern "C" uintptr_t kernel_virtual_base()
{
    return 0;
}



extern "C" void __cxa_pure_virtual()
{
    // limine_writer.writeV(core::Str("Pure virtual function called!"));
    done();
}
void load_mcx(mcx::MachineContext *context)
{
    (void)context;
}


class LinuxWriter : public core::Writer
{
  public:
    constexpr virtual core::Result<void> write(const char *buf, size_t len) override
    {
        // use linux write syscall
        size_t written = 0;
        while (written < len)
        {
            size_t to_write = len - written;
            if (to_write > 4096)
                to_write = 4096;

            long ret = syscall(SYS_write, 1, (const char *)buf + written, to_write);
          //  long ret = write(1, (const char *)buf + written, to_write);
            if (ret < 0)
            {
                return {};
            }
            written += static_cast<size_t>(ret);
        }
        return {};
    }
};
// The following will be our kernel's entry point.
void _start(void)
{

    static mcx::MachineContext _mcx{};

 //   com = arch::x86::Com::initialize(arch::x86::Com::Port::COM1).unwrap();

    LinuxWriter linux_writer;
    log::provide_log_target(&linux_writer);

    log::log$("successfully started serial...");

    load_mcx(&_mcx);

    log::log$("Mcx: {}", (const mcx::MachineContext *)&_mcx);
    arch_entry(&_mcx);

}
