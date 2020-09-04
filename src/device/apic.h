#pragma once
#include <device/acpi.h>
#include <device/madt.h>
#include <int_value.h>
class apic
{
    void *apic_addr;
    MADT_table_IOAPIC **table;
    bool loaded = false;

public:
    apic();
    void set_apic_addr(uint32_t new_address);
    void init();
    void enable();
    bool isloaded();
    void EOI(); // signal end of interrupt

    void preinit_processor(uint32_t processorid);
    void init_processor(uint32_t processorid, uint64_t entry);
    uint32_t get_current_processor_id();
    uint32_t read(uint32_t regs);
    void write(uint32_t regs, uint32_t val);
    void ipi_write(uint64_t val);
    static apic *the();
};
