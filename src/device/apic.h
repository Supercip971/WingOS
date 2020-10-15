#pragma once
#include <device/acpi.h>
#include <device/madt.h>
#include <int_value.h>
#define LAPIC_ENABLE (1 << 10)
enum apic_register
{
    lapic_id = 0x20,
    eoi = 0xb0,   // EOI register
    sivr = 0xf0,  // spurious interrupt vector register
    icr1 = 0x300, // interrupt command register
    icr2 = 0x310, // interrupt command register // why there are 2 same with the same name ? WHY OSDEV YOU ARE LYING LIKE THAT :sad:
    lvt_timer = 0x320,
    lint1 = 0x350, // interrupt command register
    lint2 = 0x360, // interrupt command register // why there are 2 same with the same name ? WHY OSDEV YOU ARE LYING LIKE THAT :sad:

    timer_div = 0x3E0,
    timer_init_counter = 0x380,
    timer_current = 0x390
};
extern "C" uint32_t *lapic_eoi_ptr;
struct io_apic_version_table
{
    uint8_t version;
    uint8_t reserved;
    uint8_t maximum_redirection;
    uint8_t reserved2;
};
class apic
{
    void *apic_addr;
    MADT_table_IOAPIC **table;
    MADT_table_ISO **iso_table;
    io_apic_version_table io_version_data;
    bool loaded = false;
    void set_raw_redirect(uint8_t vector, uint32_t target_gsi, uint16_t flags, int cpu, int status);

public:
    apic();
    void set_apic_addr(uint32_t new_address);
    void init();
    void enable();
    void send_ipi(uint8_t cpu, uint32_t interrupt_num);
    bool isloaded();
    void EOI(); // signal end of interrupt
    void load_interrupt_system();
    void preinit_processor(uint32_t processorid);
    void init_processor(uint32_t processorid, uint64_t entry);

    void set_redirect_irq(int cpu, uint8_t irq, int status = 0);

    uint32_t get_current_processor_id();
    uint32_t read(uint32_t regs);
    void write(uint32_t regs, uint32_t val);
    void io_write(uint64_t base, uint32_t reg, uint32_t data);

    uint32_t io_read(uint64_t base, uint32_t reg);
    void ipi_write(uint64_t val);

    uint32_t IO_get_max_redirect(uint32_t apic_id);
    static apic *the();
};
