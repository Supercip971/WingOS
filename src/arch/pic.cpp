

#include <arch/arch.h>
#include <arch/pic.h>
#include <int_value.h>
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

    uint8_t a1, a2;
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);
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

    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
    outb(PIC1_DATA, 0x0); // unmask all for apic
    outb(PIC2_DATA, 0x0);
}
