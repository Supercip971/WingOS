

#include <libcore/fmt/log.hpp>

#include "arch/x86_64/context.hpp"
#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/interrupts.hpp"

#include "hw/acpi/lapic.hpp"
#include "kernel/generic/scheduler.hpp"
#include "libcore/encourage.hpp"
uint64_t ccount;

extern uintptr_t _scheduler_impl(uintptr_t stack);

extern "C" uintptr_t interrupt_handler(uintptr_t stack)
{

    arch::amd64::StackFrame const *frame = reinterpret_cast<arch::amd64::StackFrame *>(stack);

    if (frame->interrupt_number < 32)
    {

        log::log$("interrupt error: n°{} - {}", frame->interrupt_number, arch::amd64::interrupts_names[frame->interrupt_number]);

        // fixme: use a real number generator for 'funny' messages instead of this
        {
            ccount *= 2;
            ccount += 3;
            uint64_t uid = frame->interrupt_number ^ frame->error_code ^ frame->rax ^ frame->rsp ^ frame->rip ^ ccount;

            uid = uid % (sizeof(core::isnt_encouraging_messages) / sizeof(core::isnt_encouraging_messages[0]));
            log::log$("-> '{}'", core::isnt_encouraging_messages[uid]);
        }

        log::log$("{}", *frame);

        uintptr_t cr2 = 0;
        asm volatile("mov %%cr2, %0"
                     : "=r"(cr2));
        log::log$("cr2: {}", cr2 | fmt::FMT_HEX | fmt::FMT_CYAN | fmt::FMT_PAD_8BYTES | fmt::FMT_PAD_ZERO);
        while (true)
        {
            asm volatile("hlt");
        }
    }
    else
    {

        _scheduler_impl(stack);
        hw::acpi::Lapic::the().eoi();
    }

    return stack;
}