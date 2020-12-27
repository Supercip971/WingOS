

#include <arch.h>
#include <int_value.h>
#include <pic.h>

void pic_init()
{
    log("pic", LOG_DEBUG) << "loading PIC";
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

    outb(PIC1_DATA, 1);
    pic_wait();
    outb(PIC2_DATA, 1);
    pic_wait();

    log("pic", LOG_INFO) << "turning off PIC for loading";
    outb(PIC1_DATA, 0xff); // mask all for apic
    pic_wait();
    outb(PIC2_DATA, 0xff);
}

void pic_ack(int intno)
{
    if (intno >= 40)
    {
        outb(PIC2_OFFSET, 0x20);
    }

    outb(PIC1_OFFSET, 0x20);
}
