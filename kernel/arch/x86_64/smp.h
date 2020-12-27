#pragma once
#include <device/madt.h>
#include <gdt.h>
#include <lock.h>
class smp
{

    void init_cpu_trampoline();

    void init_cpu_future_value(uint64_t id);

public:
    static const int max_cpu = 64;
    tss cpu_tss[max_cpu];
    smp();
    void wait();
    void init();
    void init_cpu(int apic, int id);

    uint32_t processor_count = 1;
    // we need to do the APIC
    static smp *the();

    MADT_table_LAPIC *mt_lapic[max_cpu];
};
