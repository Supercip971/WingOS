#include <arch.h>
#include <gdt.h>

#include <device/apic.h>
#include <device/local_data.h>
#include <device/time/pit.h>
#include <kernel.h>
#include <logging.h>
#include <mem/virtual.h>
#include <physical.h>
#include <proc/process.h>
#include <smp.h>
#include <sse.h>
#include <utility.h>
#include <utils/memory/liballoc.h>

#define TRAMPOLINE_START 0x1000
#define SMP_MAP_PAGE_FLAGS 0x7

smp main_smp;
int cpu_counter = 0;

extern "C" uintptr_t start_cpu_entry;
extern "C" uintptr_t end_cpu_entry;
extern "C" uint32_t trampoline_start, trampoline_end, nstack;

volatile bool SMPloaded = false;

smp::smp()
{
}

extern "C" void cpuupstart(void)
{
    init_sse();
    x86_wrmsr(MSR_REGISTERS::APIC, (x86_rdmsr(MSR_REGISTERS::APIC) | 0x800) & ~(LAPIC_ENABLE));
    apic::the()->enable();

    // set current cpu id in fs
    asm volatile(
        "mov fs, %0" ::"r"(apic::the()->get_current_processor_id()));

    log("smp", LOG_INFO, "after loading cpu: {}", apic::the()->get_current_processor_id());
    gdt_ap_init(); // create a new gdt for the cpu
    asm("cli");

    SMPloaded = true;
    asm("sti");
    while (true)
    {
    }
}

void smp::init()
{
    SMPloaded = false;
    log("smp", LOG_DEBUG, "loading smp");
    memzero(cpu_tss, sizeof(tss) * max_cpu);
    for (unsigned int i = 0; i < max_cpu; i++)
    {
        cpu_lapic_entry[i] = 0x0;
    }

    MADT_record_table_entry *mrte = madt::the()->get_madt_table_record();

    processor_count = 0;
    while ((uintptr_t)(mrte) < madt::the()->get_madt_table_lenght())
    {
        mrte = (MADT_record_table_entry *)(((uintptr_t)mrte) + mrte->tlenght);
        if (mrte->ttype == MADT_type::MADT_LAPIC)
        {
            auto local_apic = reinterpret_cast<MADT_table_LAPIC *>(mrte);

            cpu_lapic_entry[processor_count] = local_apic;
            get_current_cpu(processor_count)->lapic_id = cpu_lapic_entry[processor_count]->apic_id;

            processor_count++;
        }
    }

    log("smp", LOG_INFO, "total processor count: {}", processor_count);
    if (processor_count > max_cpu)
    {
        log("smp", LOG_WARNING, "too much processor count we will use only: {} cpu", max_cpu - 1);
    }

    for (uint64_t i = 0; i < processor_count; i++)
    {
        log("smp", LOG_INFO, "loading processor: {}", cpu_lapic_entry[i]->processor_id);
        if (apic::the()->get_current_processor_id() != cpu_lapic_entry[i]->processor_id)
        {
            init_cpu(cpu_lapic_entry[i]->apic_id, cpu_lapic_entry[i]->processor_id);
        }
    }
}
// to do : use the pit or anything else instead of this
void smp::wait()
{
}

void smp::init_cpu_trampoline()
{
    uint64_t trampoline_len = (uintptr_t)&trampoline_end - (uintptr_t)&trampoline_start;
    map_page(0, 0, true, true);

    for (uint64_t i = 0; i < (trampoline_len / PAGE_SIZE) + 2; i++)
    {
        map_page(TRAMPOLINE_START + (i * PAGE_SIZE), TRAMPOLINE_START + (i * PAGE_SIZE), true, true);
    }

    update_paging();
    memcpy((void *)TRAMPOLINE_START, &trampoline_start, trampoline_len);
}

void smp::init_cpu_future_value(uint64_t id)
{
    get_current_cpu(id)->cpu_page_table = get_current_cpu()->cpu_page_table; // give the same

    POKE((smp_cpu_init_address::PAGE_TABLE)) =
        get_rmem_addr(get_current_cpu(id)->cpu_page_table);

    memzero(get_current_cpu(id)->stack_data, get_current_cpu(id)->stack_size);

    POKE((smp_cpu_init_address::STACK)) =
        (uint64_t)get_current_cpu(id)->stack_data + get_current_cpu(id)->stack_size;

    // gdt at 0x580
    // idt at 0x590
    asm volatile(" \n"
                 "sgdt [0x580]\n"
                 "sidt [0x590]\n");

    // start address at 0x520
    POKE((smp_cpu_init_address::START_ADDR)) = (uintptr_t)&cpuupstart;
}

void smp::init_cpu(int apic, int id)
{

    log("smp cpu", LOG_DEBUG, "loading smp cpu: {} with apic id: {}", id, apic);

    memzero(get_current_cpu(id)->fpu_data, sizeof(get_current_cpu(id)->fpu_data));
    apic::the()->preinit_processor(apic);

    get_current_cpu(id)->lapic_id = apic;

    init_cpu_trampoline();
    init_cpu_future_value(id);

    log("smp cpu", LOG_INFO, " starting cpu: {}", id);

    apic::the()->init_processor(apic, TRAMPOLINE_START);

    while (SMPloaded != true)
    {
        wait();
    }

    log("smp cpu", LOG_DEBUG, " started cpu: {}", id);

    SMPloaded = false;
}

smp *smp::the()
{
    return &main_smp;
}
