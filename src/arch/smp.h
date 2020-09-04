#pragma once
#include <arch/gdt.h>
#include <device/madt.h>
class smp
{

public:
    static const int max_cpu = 64;
    smp();
    void init();
    void init_cpu(int apic, int id);
    // we need to do the APIC
    static smp *the();

private:
    tss_t cpu_tss[max_cpu];
    MADT_table_LAPIC *mt_lapic[max_cpu];

    int processor_count = 1;
};
