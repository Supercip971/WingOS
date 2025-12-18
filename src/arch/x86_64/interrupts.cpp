

#include <libcore/fmt/log.hpp>

#include "arch/x86_64/context.hpp"
#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/interrupts.hpp"
#include "arch/x86_64/registers.hpp"

#include "hw/acpi/lapic.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/scheduler.hpp"
#include "libcore/encourage.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/lock/rwlock.hpp"
uint64_t ccount;
volatile bool inside_error = false;
extern uintptr_t _scheduler_impl(uintptr_t stack);
extern uintptr_t _scheduler_impl_soft(uintptr_t stack);

core::RWLock int_lock = {};
void interrupt_release();
struct stackframe
{
    stackframe *rbp;
    uint64_t rip;
} __attribute__((packed));

void dump_stackframe(void *rbp)
{

    stackframe *frame = reinterpret_cast<stackframe *>(rbp);
    int size = 0;
    while (frame && size++ < 20)
    {
        auto rip = frame->rip;
        log::log$("stackframe: {}", rip | fmt::FMT_HEX);
        frame = frame->rbp;
    }

    if (size >= 20)
    {
        log::log$("... (stackframe too deep)");
    }
}

extern "C" uintptr_t interrupt_handler(uintptr_t stack)
{

    Cpu::current()->interrupt_hold();

    int_lock.read_acquire();

    if (Cpu::current()->in_interrupt())
    {
        log::warn$("already in an interrupt {}", Cpu::currentId());
    }

    Cpu::current()->in_interrupt(true);
    arch::amd64::StackFrame *frame = reinterpret_cast<arch::amd64::StackFrame *>(stack);

    if (frame->interrupt_number < 32)
    {
        int_lock.read_release();

        if (!Cpu::current()->in_interrupt())
        {

            int_lock.write_acquire();
        }
        while (inside_error)
        {
            asm volatile("pause");
        }
        inside_error = true;

        log::log$("cpu: {}", hw::acpi::Lapic::the().id());
        log::log$("stackframe: {}", (uint64_t)frame | fmt::FMT_HEX);
        auto interrupt_number = frame->interrupt_number;
        log::log$("interrupt error: n°{} - {}", interrupt_number, arch::amd64::interrupts_names[interrupt_number]);

        // fixme: use a real number generator for 'funny' messages instead of this
        {
            ccount *= 2;
            ccount += 3;
            auto error_code = frame->error_code;
            auto rax = frame->rax;
            auto rsp = frame->rsp;
            auto rip = frame->rip;
            uint64_t uid = interrupt_number ^ error_code ^ rax ^ rsp ^ rip ^ ccount;

            uid = uid % (sizeof(core::isnt_encouraging_messages) / sizeof(core::isnt_encouraging_messages[0]));
            log::log$("-> '{}'", core::isnt_encouraging_messages[uid]);
        }

        log::log$("{}", *(arch::amd64::StackFrame *)frame);

        uintptr_t cr2 = 0;
        asm volatile("mov %%cr2, %0"
                     : "=r"(cr2));
        log::log$("cr2: {}", cr2 | fmt::FMT_HEX | fmt::FMT_CYAN | fmt::FMT_PAD_8BYTES | fmt::FMT_PAD_ZERO);

        uintptr_t cr3 = arch::CpuCr<3>::read();
        log::log$("cr3: {}", cr3 | fmt::FMT_HEX | fmt::FMT_CYAN | fmt::FMT_PAD_8BYTES | fmt::FMT_PAD_ZERO);

        log::log$("cpu: {}", hw::acpi::Lapic::the().id());

        // dump_stackframe((void*)Cpu::current()->currentTask()->cpu_context()->);
        log::log$("kernel stacktrace:");
        dump_stackframe((void *)frame->rbp);
        log::log$("last syscall stacktrace:");

        if (Cpu::current()->debug_saved_syscall_stackframe != 0)
        {

            dump_stackframe((void *)Cpu::current()->debug_saved_syscall_stackframe);
        }

        kernel::dump_current_running_task();

        inside_error = false;
        int_lock.write_release();
        while (true)
        {
            asm volatile("hlt");
        }
    }
    else if (frame->interrupt_number == 32)
    {
        _scheduler_impl(stack);
    }
    else if (frame->interrupt_number == 100)
    {
        // log::log$("rescheduling on cpu {}", hw::acpi::Lapic::the().id());
        _scheduler_impl(stack);
    }
    else if (frame->interrupt_number == 101)
    {

        // log::log$("soft rescheduling on cpu {}", hw::acpi::Lapic::the().id());

        _scheduler_impl_soft(stack);
    }

    Cpu::current()->in_interrupt(false);
    hw::acpi::Lapic::the().eoi();

    Cpu::current()->interrupt_release(false);

    int_lock.read_release();
    return stack;
}