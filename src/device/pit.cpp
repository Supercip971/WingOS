#include <arch/arch.h>
#include <com.h>
#include <device/apic.h>
#include <device/pit.h>
#include <kernel.h>
#pragma GCC optimize("-O0")
PIT global_PIT;
extern "C" void pit_callback()
{
    PIT::the()->update();
}
void PIT::init_PIT()
{
    *this = PIT();
    com_write_str("loading PIT");
    uint16_t divisor = PIT_START_FREQUENCY /
                       PIT_TARGET_FREQUECY; // to do : make this more portable

    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    wait();
    outb(0x40, l);
    wait();
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);
    outb(0x40, h);
    com_write_str("loaded PIT");
}
void PIT::Pwait(uint16_t ms)
{
    outb(0x43, 0x30);
    uint16_t wait_val = PIT_START_FREQUENCY / 1000;
    wait_val *= ms;

    uint8_t l = (uint8_t)(wait_val & 0xFF);
    wait();
    outb(0x40, l);
    wait();
    uint8_t h = (uint8_t)((wait_val >> 8) & 0xFF);
    outb(0x40, h);

    while (true)
    {
        outb(0x43, 0xe2);
        uint8_t status = inb(0x40);
        if ((status & (1 << 7)) != 0)
        {
            break;
        }
    }
}
void PIT::update()
{
    apic::the()->EOI();
    total_count++;
    current_count++;
    if (current_count > PIT_TARGET_FREQUECY)
    {
        com_write_str("sec");
        current_count = 0;
        passed_sec += 1;
    }
}
bool loaded = false;

PIT *PIT::the() { return &global_PIT; }
