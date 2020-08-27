#include <arch/64bit.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/process.h>
#include <com.h>
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

#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_OFFSET 0x20
#define PIC1_DATA (PIC1 + 1)

#define PIC2 0xA0
#define PIC2_COMMAND PIC2
#define PIC2_OFFSET 0x28
#define PIC2_DATA (PIC2 + 1)

#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10
#define pic_wait()                    \
    do                                \
    {                                 \
        asm volatile("jmp 1f\n\t"     \
                     "1:\n\t"         \
                     "    jmp 2f\n\t" \
                     "2:");           \
    } while (0)

void pic_init()
{
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    pic_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    pic_wait();

    outb(PIC1_DATA, PIC1_OFFSET);
    pic_wait();
    outb(PIC2_DATA, PIC2_OFFSET);
    pic_wait();

    outb(PIC1_DATA, 0x04);
    pic_wait();
    outb(PIC2_DATA, 0x02);
    pic_wait();

    outb(PIC1_DATA, 0x01);
    pic_wait();
    outb(PIC2_DATA, 0x01);
    pic_wait();

    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}
void init_idt()
{
    com_write_str("loading idt");
    com_write_str("loading idt table");
    for (int i = 0; i < 32 + 48; i++)
    {
        idt[i] = IDT_ENTRY(__interrupt_vector[i], 0x08, INTGATE);
    }

    com_write_str("loading idt idt_flush");
    asm volatile("lidt [%0]"
                 :
                 : "m"(idt_descriptor));
    com_write_str("loading pic");
    pic_init();
    com_write_str("loading pic : OK");
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
    com_write_str(" ===== cpu dump =====");
    com_write_str(" ===== cs and ss =====");
    dump1(stck->cs, "cs");
    dump1(stck->ss, "ss");
    com_write_str(" ");
    com_write_str(" ===== rx =====");
    dump1(stck->r8, "r8");
    dump1(stck->r9, "r9");
    dump1(stck->r10, "r10");
    com_write_str(" ");
    dump1(stck->r11, "r11");
    dump1(stck->r12, "r12");
    dump1(stck->r13, "r13");
    com_write_str(" ");
    dump1(stck->r14, "r14");
    dump1(stck->r15, "r15");
    com_write_str(" ");
    com_write_str(" ===== utility =====");
    dump1(stck->rsp, "rsp");
    dump1(stck->rbp, "rbp");
    dump1(stck->rdi, "rdi");
    com_write_str(" ");
    dump1(stck->rsi, "rsi");
    dump1(stck->rdx, "rdx");
    dump1(stck->rcx, "rcx");
    com_write_str(" ");
    dump1(stck->rbx, "rbx");
    dump1(stck->rax, "rax");
    com_write_str(" ");
    com_write_str(" ===== other =====");
    dump1(stck->error_code, "error_code");
    dump1(stck->int_no, "int_no");
    com_write_str(" ");
    dump1(stck->rip, "rip");
    dump1(stck->rflags, "rflags");
    com_write_str(" ");
    com_write_str(" ===== CRX =====");

    uint64_t CRX;
    asm volatile("mov %%cr2, %0"
                 : "=r"(CRX));
    dump1(CRX, "CR2");
}
void pic_ack(int intno)
{
    if (intno >= 40)
    {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
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

        memzero(buff, 64);
        kitoa(buff, 'd', stackframe->int_no);

        com_write_str("id :");
        com_write_str(buff);
        com_write_str("error fatal");
        com_write_str(exception_messages[stackframe->int_no]);
        dumpregister(stackframe);
        memzero(buff, 64);
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
        }
        while (true)
        {
        }
    }
    if (stackframe->int_no == 32)
    {
        PIT::the()->update();
        irq_0_process_handler(stackframe);
    }

    pic_ack(stackframe->int_no);
}
