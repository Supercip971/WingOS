#include <64bit.h>
#include <arch.h>
#include <com.h>
#include <device/apic.h>
#include <device/ata_driver.h>
#include <device/local_data.h>
#include <device/network/e1000.h>
#include <device/pit.h>
#include <device/ps_keyboard.h>
#include <device/ps_mouse.h>
#include <gdt.h>
#include <interrupt.h>
#include <kernel.h>
#include <logging.h>
#include <pic.h>
#include <process.h>
#include <smp.h>
#include <syscall.h>
#include <utility.h>
#include <utils/lock.h>

extern utils::lock_type log_locker;
struct interrupt_handler_specific_array
{
    irq_handler_func function_list[8]; // max 8 handler for an irq;
    int current_array_length = 0;
};

interrupt_handler_specific_array irq_array_handler[32];

const char *interrupt_exception_name[] = {
    "Division By 0",
    "Debug interrupt",
    "NMI (Non Maskable Interrupt)",
    "Breakpoint interrupt",
    "invalid (4)", // int 4 is not valid in 64 bit
    "table overflow",
    "Invalid opcode",
    "No FPU",
    "Double fault",
    "invalid (9)", // int 9 is not used
    "invalid TSS",
    "Segment not present",
    "invalid stack",
    "General protection fault",
    "Page fault",
    "invalid (15)",
    "x87 FPU fault",
    "Alignment fault",
    "Machine check fault",
    "SIMD floating point exception",
    "vitualisation excpetion",
    "control protection exception",
    "invalid (22)",
    "invalid (23)",

    "invalid (24)",
    "invalid (25)",
    "invalid (26)",
    "invalid (27)",
    "invalid (28)",
    "invalid (29)",
    "invalid (30)",
    "invalid (31)"};

extern uintptr_t __interrupt_vector[128];
idt_entry idt[IDT_ENTRY_COUNT];

idtr idt_descriptor = {
    .size = sizeof(idt_entry) * IDT_ENTRY_COUNT,
    .offset = (uint64_t)&idt[0],
};

ASM_FUNCTION void idt_flush(uint64_t);
ASM_FUNCTION void syscall_asm_entry();

void add_irq_handler(irq_handler_func func, uint8_t irq_target)
{
    if (irq_target <= 32)
    {
        interrupt_handler_specific_array *target = &irq_array_handler[irq_target];
        if (target->current_array_length < 7)
        {
            target->function_list[target->current_array_length] = func;
            target->current_array_length++;
            return;
        }
    }
    log("int", LOG_ERROR) << "can't add irq id " << irq_target;
}

void call_irq_handlers(unsigned int irq, InterruptStackFrame *isf)
{
    interrupt_handler_specific_array &target = irq_array_handler[irq];
    if (target.current_array_length == 0)
    {
        return;
    }
    for (int i = 0; i < target.current_array_length; i++)
    {
        target.function_list[i](irq);
    }
}

void init_irq_handlers()
{
    for (int i = 0; i < 32; i++)
    {
        irq_array_handler[i].current_array_length = 0;
    }
}

void init_idt()
{

    log("idt", LOG_INFO) << "loading idt table";
    for (int i = 0; i < 32 + 48; i++)
    {
        idt[i] = idt_entry((void *)__interrupt_vector[i], 0, INTGATE);
    }
    idt[127] = idt_entry((void *)__interrupt_vector[48], 0, INTGATE | INT_USER);
    idt[100] = idt_entry((void *)__interrupt_vector[49], 0, INTGATE | INT_USER);

    log("idt", LOG_DEBUG) << "flushing idt";
    init_irq_handlers();
    idt_flush((uint64_t)&idt_descriptor);
};

bool is_interrupt_error(uint8_t intno)
{
    if (intno > 31)
    {
        return false;
    }
    // yeah i should do an array
    if (intno == 1 || intno == 15 || (intno >= 21 && intno <= 29) ||
        intno == 31)
    {
        return false;
    }
    return true;
}

// backtrace

void update_backtrace(InterruptStackFrame *stackframe)
{
    process::current()->get_backtrace().add_entry(stackframe->rip);
    get_current_cpu()->local_backtrace.add_entry(stackframe->rip);
}

bool error = false;
void interrupt_error_handle(InterruptStackFrame *stackframe)
{
    error = true;
    log("pic", LOG_FATAL, "!!! fatal interrupt error !!! {}",stackframe->rip);
    log("pic", LOG_ERROR, "ID   : {}", stackframe->int_no);
    log("pic", LOG_ERROR, "type : {}",interrupt_exception_name[stackframe->int_no]);

    printf("\n");

    dump_register(stackframe);

    printf("\n");

    log("pic", LOG_ERROR) << "current process backtrace : ";
    process::current()->get_backtrace().dump_backtrace();
    log("pic", LOG_ERROR) << "current cpu backtrace :";
    get_current_cpu()->local_backtrace.dump_backtrace();

    if (process::current() != nullptr)
    {
        log("pic", LOG_INFO) << "in process: " << process::current()->get_name();
        log("pic", LOG_INFO) << "in processor : " << process::current()->get_cpu();
        dump_process();
    }
    while (true)
    {
        halt_interrupt();
    }
}

ASM_FUNCTION uintptr_t interrupts_handler(InterruptStackFrame *stackframe)
{

    uintptr_t nresult = reinterpret_cast<uintptr_t>(stackframe);
    update_backtrace(stackframe);
    if (error)
    {
        while (error)
            ;
    }
    if (is_interrupt_error(stackframe->int_no))
    {
        log_locker.unlock();
        interrupt_error_handle(stackframe);
    }

    if (stackframe->int_no == 0x7f)
    {
        stackframe->rax = syscall(stackframe->rax, stackframe->rbx, stackframe->rcx, stackframe->rdx, stackframe->rsi, stackframe->rdi, stackframe); // don't use r11 for future use with x64 syscalls
    }
    else if (stackframe->int_no == 32)
    {
        nresult = process_switch_handler((arch_stackframe *)stackframe, true);
    }
    else if (stackframe->int_no > 32 && stackframe->int_no < 64)
    {
        call_irq_handlers(stackframe->int_no - 32, stackframe);
    }
    else if (stackframe->int_no == 100)
    {
        nresult = process_switch_handler((arch_stackframe *)stackframe, false);
    }
    else if (stackframe->int_no == 0xf0)
    {
        printf("apic : Nmi : possible hardware error :( \n");
    }

    apic::the()->EOI();

    return nresult;
}
