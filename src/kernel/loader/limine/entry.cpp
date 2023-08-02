
#include <arch/x86/com.hpp>
#include <kernel/generic/kernel.hpp>
#include <kernel/loader/limine/limine.hpp>
#include <libcore/fmt/fmt.hpp>
#include <stddef.h>
#include <stdint.h>

#include "libcore/fmt/log.hpp"
#include "libcore/io/writer.hpp"
#include "libcore/str.hpp"
#include "mcx/mcx.hpp"

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

void _start(void);

__attribute__((used)) static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

__attribute__((used)) static volatile struct limine_entry_point_request entry_point_request = {
    .id = LIMINE_ENTRY_POINT_REQUEST,
    .revision = 0,
    .entry = _start,
};

static void done(void)
{
    for (;;)
    {
        __asm__("hlt");
    }
}

class LimineWriter : public core::Writer
{
public:
    core::Result<void> write(const char *data, size_t size) override
    {

        struct limine_terminal *terminal = terminal_request.response->terminals[0];
        terminal_request.response->write(terminal, data, size);
        return {};
    }
    template <core::Viewable T>
    constexpr core::Result<void> write(T view)
        requires(core::Viewable<T>)
    {
        return write(view.data(), view.len());
    }
};

extern "C" void __cxa_pure_virtual()
{
    // limine_writer.writeV(core::Str("Pure virtual function called!"));
    done();
}
void load_mcx(mcx::MachineContext *context);

// The following will be our kernel's entry point.
void _start(void)
{

    static LimineWriter limine_writer{};

    static mcx::MachineContext _mcx{};
    static arch::x86::Com com{};
    asm volatile("cli");
    limine_writer = LimineWriter();
    log::provide_log_target(&limine_writer);

    com = arch::x86::Com::initialize(arch::x86::Com::Port::COM1).unwrap();

    log::provide_log_target(&com);

    log::log$("successfully started serial...");

    load_mcx(&_mcx);

    log::log$("Mcx: {}", (const mcx::MachineContext *)&_mcx);
    arch_entry(&_mcx);
    done();
}