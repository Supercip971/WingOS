#pragma once
#include <device/madt.h>
#include <gdt.h>
#include <utils/config.h>

enum smp_cpu_init_address
{
    PAGE_TABLE = 0x500,
    START_ADDR = 0x520,
    STACK = 0x570,
    GDT = 0x580,
    IDT = 0x590,
};

class smp
{

    void init_cpu_trampoline();

    void init_cpu_future_value(uint64_t id);

public:
    static const unsigned int max_cpu = MAX_CPU_COUNT;
    tss cpu_tss[max_cpu];
    smp();
    void wait();
    void init();
    void init_cpu(int apic, int id);

    uint32_t processor_count = 1;
    // we need to do the APIC
    static smp *the();

    MADT_table_LAPIC *cpu_lapic_entry[max_cpu];
};
