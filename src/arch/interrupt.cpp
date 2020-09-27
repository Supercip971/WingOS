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
#include <device/pit.h>
#include <kernel.h>
#include <loggging.h>
#include <syscall.h>
#include <utility.h>
uint8_t current = 0;

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
static idt_entry_t idt[IDT_ENTRY_COUNT];

static idtr_t idt_descriptor = {
    .size = sizeof(idt_entry_t) * IDT_ENTRY_COUNT,
    .offset = (uint64_t)&idt[0],
};

uint64_t rip_count[16];
uint32_t rip_counter = 0;

uint64_t rip_backtrace[32];
extern "C" void idt_flush(uint64_t);
extern "C" void syscall_asm_entry();

static idt_entry_t register_interrupt_handler(void *handler, uint8_t ist, uint8_t type)
{
    uint64_t p = (uint64_t)handler;
    idt_entry_t idt;
    idt.offset_low16 = (uint16_t)p;
    idt.cs = SLTR_KERNEL_CODE;
    idt.ist = ist;
    idt.attributes = type;
    idt.offset_mid16 = (uint16_t)(p >> 16);
    idt.offset_high32 = (uint32_t)(p >> 32);
    idt.zero = 0;

    return idt;
}
__attribute__((interrupt)) static void nmi_handler(void *p)
{
    log("apic", LOG_ERROR) << "nmi interrupt";
    apic::the()->EOI();
}
__attribute__((interrupt)) static void spurious_handler(void *p)
{

    log("apic", LOG_ERROR) << "spurrious interrupt";
    apic::the()->EOI();
}
__attribute__((interrupt)) static void too_much_handler(InterruptStackFrame *p)
{
    log("apic", LOG_ERROR) << "not handled interrupt";

    // printf("## [ apic ] : not handled interrupt \n");

    // printf("un-interrupt %x", p->int_no);

    apic::the()->EOI();
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
    for (int i = 32 + 48; i < 0xff; i++)
    {
        //   idt[i] = register_interrupt_handler((void *)__interrupt_vector[i], 0, 0x8e);

        idt[i] = register_interrupt_handler((void *)too_much_handler, 0, 0x8e);
    }
    idt[127] = register_interrupt_handler((void *)__interrupt_vector[48], 0, 0x8e);
    idt[100] = register_interrupt_handler((void *)__interrupt_vector[49], 0, 0x8e);

    idt[0xf0] = register_interrupt_handler((void *)nmi_handler, 0, 0x8e);
    idt[0xff] = register_interrupt_handler((void *)spurious_handler, 0, 0x8e);
    log("idt", LOG_DEBUG) << "flushing idt";

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

    uint64_t CRX;
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

void pic_ack(int intno)
{
    if (intno >= 40)
    {
        outb(PIC2_OFFSET, 0x20);
    }

    outb(PIC1_OFFSET, 0x20);
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

extern "C" uint64_t interrupts_handler(InterruptStackFrame *stackframe)
{
    //lock((&lock));
    if (rip_backtrace[32] != stackframe->rip)
    {
        for (int i = 0; i <= 31; i++)
        {
            rip_backtrace[i] = rip_backtrace[i + 1];
        }
        rip_backtrace[32] = stackframe->rip;
    }
    if (stackframe->int_no != 0x24 && stackframe->int_no != 0x22 && stackframe->int_no != 32 && stackframe->int_no != 0x2e)
    {
        if (stackframe->int_no == 0x7f)
        {
            stackframe->rax = syscall(stackframe->rax, stackframe->rbx, stackframe->rcx, stackframe->rdx, stackframe->rsi, stackframe->rdi); // don't use r11 for future use with x64 syscalls

            apic::the()->EOI();
            return (uint64_t)stackframe;
        }
        else
        {
            // i don't remove this one
            //    printf("\n interrupt %x in processor %x \n", stackframe->int_no, apic::the()->get_current_processor_id());
        }
    }
    if (is_error(stackframe->int_no))
    {

        log("pic", LOG_FATAL) << "!!! fatal interrupt error !!!";
        log("pic", LOG_ERROR) << "ID   : " << stackframe->int_no;
        log("pic", LOG_ERROR) << "type : " << exception_messages[stackframe->int_no];

        printf("\n");
        dumpregister(stackframe);
        printf("\n");
        log("pic", LOG_ERROR) << "backtrace : ";
        for (int i = 0; i <= 32; i++)
        {
            if (rip_backtrace[i] != -32)
            {
                log("pic", LOG_ERROR) << "id " << i << " = " << rip_backtrace[i];
            }
        }

        if (get_current_data()->current_process != nullptr)
        {
            log("pic", LOG_INFO) << "in process: " << get_current_data()->current_process->process_name;

            log("pic", LOG_INFO) << "in processor : " << get_current_data()->current_process->processor_target;
            dump_process();
        }
        while (true)
        {
            asm volatile("hlt");
        }
    }
    else if (stackframe->int_no == 32)
    {
        PIT::the()->update();
        uint64_t result = irq_0_process_handler(stackframe);
        apic::the()->EOI();
        return result;
    }
    else if (stackframe->int_no == 33)
    {
        unsigned char scan_code = inb(0x60);
    }
    else if (stackframe->int_no == 32 + 14 || stackframe->int_no == 32 + 15)
    {
        ata_driver::the()->irq_handle(stackframe->int_no - 32);
    }

    else if (stackframe->int_no == 100)
    {

        uint64_t result = irq_0_process_handler(stackframe);
        apic::the()->EOI();
        return result;
    }
    else if (stackframe->int_no == 0xf0)
    {
        printf("apic : Nmi : possible hardware error :( \n");
    }
    else if (stackframe->int_no == 0xff)
    {
    }

    apic::the()->EOI();
    return (uint64_t)stackframe;
    // unlock((&lock));
}
