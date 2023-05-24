#include "arch/x86_64/idt.hpp"
#include "kernel/kernel.hpp"

#include <libcore/fmt/log.hpp>
extern "C" uintptr_t interrupt_handler(uintptr_t stack)
{

    log::log$("interrupt!");

    return stack;
}