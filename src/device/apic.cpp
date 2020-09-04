#include <arch/arch.h>
#include <arch/mem/virtual.h>
#include <arch/pic.h>
#include <com.h>
#include <device/acpi.h>
#include <device/apic.h>
apic main_apic = apic();
enum apic_register
{
    lapic_id = 0x20,
    eoi = 0xb0,   // EOI register
    sivr = 0xf0,  // spurious interrupt vector register
    icr1 = 0x300, // interrupt command register
    icr2 = 0x310, // interrupt command register // why there are 2 same with the same name ? WHY OSDEV YOU ARE LYING LIKE THAT :sad:

};

apic::apic()
{
    loaded = false;
}

void apic::enable()
{
    write(sivr, read(sivr) | 0x100);
}
void apic::EOI()
{
    write(eoi, 0);
}

bool apic::isloaded()
{
    return loaded;
}
void apic::init()
{
    com_write_str("loading apic");
    table = madt::the()->get_madt_ioAPIC();
    apic_addr = (void *)get_mem_addr(madt::the()->madt_header->lapic);
    if (apic_addr == nullptr)
    {
        com_write_str("[error] can't find apic (sad)");
        while (true)
        {
            asm("hlt");
        }
        return;
    }
    x86_wrmsr(0x1B, x86_rdmsr(0x1B) | 0x800);
    enable();
    com_write_str("loading apic : OK");
    com_write_reg("current processor id ", get_current_processor_id());
    loaded = true;
}

uint32_t apic::read(uint32_t regs)
{
    return *((volatile uint32_t *)((uint64_t)apic_addr + regs));
}

void apic::write(uint32_t regs, uint32_t val)
{
    *((volatile uint32_t *)((uint64_t)apic_addr + regs)) = val;
}
void apic::set_apic_addr(uint32_t new_address)
{
    apic_addr = (void *)new_address;
    // not used
}
apic *apic::the()
{
    return &main_apic;
}
uint32_t apic::get_current_processor_id()
{
    return (read(lapic_id) >> 24);
}
void apic::preinit_processor(uint32_t processorid)
{

    write(icr2, (processorid << 24));
    write(icr1, 0x500);
}

void apic::init_processor(uint32_t processorid, uint64_t entry)
{

    write(icr2, (processorid << 24));
    write(icr1, 0x600 | ((uint32_t)entry / 4096));
}
