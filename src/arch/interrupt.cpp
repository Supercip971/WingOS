#include <arch/64bit.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/pic.h>
#include <arch/process.h>
#include <com.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <device/pit.h>
#include <kernel.h>
#include <utility.h>

#define IDT_ENTRY(__offset, __selector, __type)                              \
    (idt_entry_t)                                                            \
    {                                                                        \
        .offset_low16 = (uint16_t)((__offset)&0xffff), .cs = (__selector),   \
        .ist = 0, .attributes = (__type),                                    \
        .offset_mid16 = ((__offset & 0xFFFF0000) >> 16),                     \
        .offset_high32 = ((__offset & 0xFFFFFFFF00000000) >> 32), .zero = 0, \
    }
static idt_entry_t register_interrupt_handler(void *handler, uint8_t ist, uint8_t type)
{
    uint64_t p = (uint64_t)handler;
    idt_entry_t idt;
    idt.offset_low16 = (uint16_t)p;
    idt.cs = 0x08;
    idt.ist = ist;
    idt.attributes = type;
    idt.offset_mid16 = (uint16_t)(p >> 16);
    idt.offset_high32 = (uint32_t)(p >> 32);
    idt.zero = 0;

    return idt;
}
#define IDT_ENTRY_COUNT 256
#define INTGATE 0x8e
#define TRAPGATE 0xeF

extern uintptr_t __interrupt_vector[128];
static idt_entry_t idt[IDT_ENTRY_COUNT];
static idtr_t idt_descriptor = {
    .size = sizeof(idt_entry_t) * IDT_ENTRY_COUNT,
    .offset = (uint64_t)&idt[0],
};
extern "C" void idt_flush(uint64_t);
extern "C" void syscall_asm_entry();

void init_idt()
{
    com_write_str("loading idt");
    com_write_str("loading idt table");
    for (int i = 0; i < 32 + 48; i++)
    {
        idt[i] = register_interrupt_handler((void *)__interrupt_vector[i], 0, 0x8e);
    }

    com_write_str("loading idt idt_flush");
    asm volatile("lidt [%0]"
                 :
                 : "m"(idt_descriptor));
    com_write_str("loading idt : OK");

    com_write_str("turning on interrupt : OK ");
};

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

char buff2[64];
void dump1(uint64_t reg, const char *name)
{
    memzero(buff2, 64);
    kitoaT<uint64_t>(buff2, 'x', reg);
    com_write_strl(" | ");
    com_write_strl(name);
    com_write_strl(" = ");
    com_write_strl("0x");
    com_write_strl(buff2);
}
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
    asm volatile("mov %%cr2, %0"
                 : "=r"(CRX));
    printf("CR2 = %x \n", CRX);
}
void pic_ack(int intno)
{
    if (intno >= 40)
    {
        outb(PIC2_OFFSET, 0x20);
    }

    outb(PIC1_OFFSET, 0x20);
}
char buff[64];
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
uint64_t rip_count[16];
uint32_t rip_counter = 0;
void add_rip(uint64_t addr)
{
    rip_count[rip_counter++] = addr;
    if (rip_counter == 15)
    {
        rip_counter = 0;
    }
}

static int dd = 0;
extern "C" void interrupts_handler(InterruptStackFrame *stackframe)
{
    add_rip(stackframe->rip);
    if (is_error(stackframe->int_no))
    {
        for (int i = 0; i < stackframe->int_no * 320; i++)
        {
            is_error(stackframe->int_no);
        }

        printf("error fatal \n");
        printf("id : %x \n", stackframe->int_no);
        printf("type : %s\n", exception_messages[stackframe->int_no]);
        dumpregister(stackframe);
        /*memzero(buff, 64);
        kitoaT<uint64_t>(buff, 'x', stackframe->rip);
        com_write_str(" ===== ");
        com_write_str("rip :");
        com_write_str(buff);
        for (uint64_t iz = rip_counter + 1; iz < 16; iz++)
        {
            memzero(buff, 64);
            kitoaT<uint64_t>(buff, 'x', rip_count[iz]);
            com_write_str("rip :");
            com_write_str(buff);
        }
        for (uint64_t iz = 0; iz <= rip_counter; iz++)
        {
            memzero(buff, 64);
            kitoaT<uint64_t>(buff, 'x', rip_count[iz]);
            com_write_str("rip :");
            com_write_str(buff);
        }*/
        while (true)
        {
        }
    }
    if (stackframe->int_no == 32)
    {
        PIT::the()->update();
        irq_0_process_handler(stackframe);
    }
    if (stackframe->int_no == 0xf0)
    {
        printf("apic : Nmi : possible hardware error :( \n");
        apic::the()->EOI();
    }
    if (stackframe->int_no == 0xff)
    {
        printf("apic : spurr \n");
        apic::the()->EOI();
    }
    pic_ack(stackframe->int_no);
}
