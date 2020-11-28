#include <arch/64bit.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/lock.h>
#include <arch/pic.h>
#include <arch/process.h>
#include <arch/smp.h>
#include <com.h>
#include <device/apic.h>
#include <device/ata_driver.h>
#include <device/local_data.h>
#include <device/network/e1000.h>
#include <device/pit.h>
#include <device/ps_keyboard.h>
#include <device/ps_mouse.h>
#include <kernel.h>
#include <logging.h>
#include <syscall.h>
#include <utility.h>
uint8_t current = 0;

struct interrupt_handler_specific_array
{
    irq_handler_func function_list[8]; // max 8 handler for an irq;
    int current_array_length = 0;
};

interrupt_handler_specific_array irq_hdlr_array[32];

lock_type lock = {0};
const char *exception_messages[] = {"Division By Zero",
                                    "Debug",
                                    "Non Maskable Interrupt",
                                    "Breakpoint",
                                    "Into Detected Overflow",
                                    "Out of Bounds",
                                    "Invalid Opcode",
                                    "No Coprocessor",

                                    "Double Fault",
                                    "Coprocessor Segment Overrun",
                                    "Bad TSS",
                                    "Segment Not Present",
                                    "Stack Fault",
                                    "General Protection Fault",
                                    "Page Fault",
                                    "Unknown Interrupt",

                                    "Coprocessor Fault",
                                    "Alignment Check",
                                    "Machine Check",
                                    "Reserved",
                                    "Reserved",
                                    "Reserved",
                                    "Reserved",
                                    "Reserved",

                                    "Reserved",
                                    "Reserved",
                                    "Reserved",
                                    "Reserved",
                                    "Reserved",
                                    "Reserved",
                                    "Reserved",
                                    "Reserved"};

extern uintptr_t __interrupt_vector[128];
static idt_entry idt[IDT_ENTRY_COUNT];

static idtr idt_descriptor = {
    .size = sizeof(idt_entry) * IDT_ENTRY_COUNT,
    .offset = (uint64_t)&idt[0],
};

uintptr_t rip_backtrace[32];

extern "C" void idt_flush(uint64_t);
extern "C" void syscall_asm_entry();

static idt_entry register_interrupt_handler(void *handler, uint8_t ist, uint8_t type)
{
    uintptr_t p = (uintptr_t)handler;
    idt_entry idt;

    idt.offset_low16 = (uint16_t)p;
    idt.cs = SLTR_KERNEL_CODE;
    idt.ist = ist;
    idt.attributes = type;
    idt.offset_mid16 = (uint16_t)(p >> 16);
    idt.offset_high32 = (uint32_t)(p >> 32);
    idt.zero = 0;

    return idt;
}

void add_irq_handler(irq_handler_func func, unsigned int irq_target)
{
    if (irq_target <= 32)
    {
        interrupt_handler_specific_array *target = &irq_hdlr_array[irq_target];
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
    interrupt_handler_specific_array &target = irq_hdlr_array[irq];
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
        irq_hdlr_array[i].current_array_length = 0;
    }
}

void init_idt()
{
    log("idt", LOG_DEBUG) << "loading idt";
    for (int i = 0; i < 32; i++)
    {
        rip_backtrace[i] = -32;
    }

    log("idt", LOG_INFO) << "loading idt table";
    for (int i = 0; i < 32 + 48; i++)
    {
        idt[i] = register_interrupt_handler((void *)__interrupt_vector[i], 0, 0x8e);
    }
    idt[127] = register_interrupt_handler((void *)__interrupt_vector[48], 0, 0x8e);
    idt[100] = register_interrupt_handler((void *)__interrupt_vector[49], 0, 0x8e);

    log("idt", LOG_DEBUG) << "flushing idt";
    init_irq_handlers();
    asm volatile("lidt [%0]"
                 :
                 : "m"(idt_descriptor));
};

void dumpregister(InterruptStackFrame *stck)
{
    // this is the least readable code EVER
    printf(" ===== cpu dump ===== \n");
    printf(" ===== cs and ss ===== \n");

    printf("cs = %x | ss = %x \n", stck->cs, stck->ss);
    printf(" ===== utility ===== \n");
    printf("rsp = %x | rbp = %x | rdi = %x \n", stck->rsp, stck->rbp, stck->rdi);
    printf("rsi = %x | rdx = %x | rcx = %x \n", stck->rsi, stck->rdx, stck->rcx);
    printf("rbx = %x | rax = %x |  \n", stck->rbx, stck->rax);
    printf(" ===== other ===== \n");
    printf("error code = %x \n", stck->error_code);
    printf("interrupt number = %x \n", stck->int_no);
    printf("rip = %x \n", stck->rip);
    printf("flags = %x \n", stck->rflags);

    uintptr_t CRX;
    asm volatile("mov %0, cr2"
                 : "=r"(CRX));
    printf("CR2 = %x \n", CRX);
    asm volatile("mov %0, cr3"
                 : "=r"(CRX));
    printf("CR3 = %x \n", CRX);
    asm volatile("mov %0, cr4"
                 : "=r"(CRX));
    printf("CR4 = %x \n", CRX);
}

bool is_error(int intno)
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
    if (get_current_cpu()->current_process != nullptr)
    {
        if (get_current_cpu()->current_process->rip_backtrace[31] != stackframe->rip)
        {
            for (int i = 0; i < 30; i++)
            {
                get_current_cpu()->current_process->rip_backtrace[i] = get_current_cpu()->current_process->rip_backtrace[i + 1];
            }
            get_current_cpu()->current_process->rip_backtrace[31] = stackframe->rip;
        }
    }
}

bool error = false;
void interrupt_error_handle(InterruptStackFrame *stackframe)
{
    error = true;
    log("pic", LOG_FATAL) << "!!! fatal interrupt error !!!" << stackframe->rip;
    log("pic", LOG_ERROR) << "ID   : " << stackframe->int_no;
    log("pic", LOG_ERROR) << "type : " << exception_messages[stackframe->int_no];

    printf("\n");

    dumpregister(stackframe);

    printf("\n");

    log("pic", LOG_ERROR) << "backtrace : ";

    for (int i = 32; i >= 0; i--)
    {
        if (get_current_cpu()->current_process != nullptr)
        {

            if (get_current_cpu()->current_process->rip_backtrace[i] != 0)
            {
                log("pic", LOG_ERROR) << "id " << i << " = " << get_current_cpu()->current_process->rip_backtrace[i];
            }
        }
    }

    if (get_current_cpu()->current_process != nullptr)
    {
        log("pic", LOG_INFO) << "in process: " << get_current_cpu()->current_process->process_name;

        log("pic", LOG_INFO) << "in processor : " << get_current_cpu()->current_process->processor_target;
        dump_process();
    }
    while (true)
    {
        asm volatile("hlt");
    }
}

extern "C" uintptr_t interrupts_handler(InterruptStackFrame *stackframe)
{

    uintptr_t nresult = reinterpret_cast<uintptr_t>(stackframe);
    update_backtrace(stackframe);
    if (error)
    {
        while (error)
            ;
    }
    if (is_error(stackframe->int_no))
    {
        locker_print.data = 0;
        print_locker.data = 0;
        interrupt_error_handle(stackframe);
    }

    if (stackframe->int_no == 0x7f)
    {
        stackframe->rax = syscall(stackframe->rax, stackframe->rbx, stackframe->rcx, stackframe->rdx, stackframe->rsi, stackframe->rdi); // don't use r11 for future use with x64 syscalls
        apic::the()->EOI();
    }
    else if (stackframe->int_no == 32)
    {
        PIT::the()->update();
        nresult = irq_0_process_handler(stackframe);
    }
    else if (stackframe->int_no > 32 && stackframe->int_no < 64)
    {
        call_irq_handlers(stackframe->int_no - 32, stackframe);
    }
    else if (stackframe->int_no == 100)
    {
        nresult = irq_0_process_handler(stackframe);
    }
    else if (stackframe->int_no == 0xf0)
    {
        printf("apic : Nmi : possible hardware error :( \n");
    }

    apic::the()->EOI();

    return nresult;
}
