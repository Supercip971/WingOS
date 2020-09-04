#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/physical.h>
#include <arch/mem/virtual.h>
#include <arch/process.h>
#include <arch/smp.h>
#include <com.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <device/pit.h>
#include <kernel.h>
#define TRAMPOLINE_START 0x1000
#define TRAMPOLINE_PAGING_ADDR 0x4000

#pragma GCC optimize("-O0")
smp main_smp;
smp::smp()
{
}
char data[] = "hello world";
bool SMPloaded = false;
extern "C" void cpuupstart(void)
{
    com_write_str("ee");
    while (true)
    {
    }
}

void test_func()
{
    while (true)
    {
        outb(0x3F8, 'e');
        int a = 0;
        for (uint64_t e = 0; e < 100; e++)
        {
            a *= e - a;
        }
    }
}
void test_func1()
{
    while (true)
    {
        com_write_str("hello 2");
    }
}
void smp::init()
{

    memzero(cpu_tss, sizeof(tss_t) * max_cpu);
    for (int i = 0; i < max_cpu; i++)
    {
        mt_lapic[i] = nullptr;
    }
    MADT_record_table_entry *mrte = madt::the()->get_madt_table_record();
    processor_count = 0;
    while (uint64_t(mrte) < madt::the()->get_madt_table_lenght())
    {
        mrte = (MADT_record_table_entry *)(((uint64_t)mrte) + mrte->tlenght);

        if (mrte->ttype == MADT_type::MADT_LAPIC)
        {

            auto local_apic = reinterpret_cast<MADT_table_LAPIC *>(mrte);

            mt_lapic[processor_count] = local_apic;
            processor_count++;
        }
    }
    processor_count--;
    com_write_reg("total processor count detected : ", processor_count);
    if (processor_count > max_cpu)
    {
        com_write_reg("too much processor are detected, we will only use : ", max_cpu - 1);
    }

    for (int i = 0; i < processor_count; i++)
    {
        com_write_reg("getting proc : ", mt_lapic[i]->processor_id);
        if (apic::the()->get_current_processor_id() != mt_lapic[i]->processor_id)
        {
            init_cpu(mt_lapic[i]->apic_id, mt_lapic[i]->processor_id);
        }
    }
}
int cpu_counter = 0;
extern "C" uint64_t start_cpu_entry;
extern "C" uint64_t end_cpu_entry;
extern "C" uint32_t trampoline_start, trampoline_end, nstack;

void smp::init_cpu(int apic, int id)
{
    com_write_reg("init cpu id : ", id);
    com_write_reg("init cpu apic id : ", apic);
    uint64_t trampoline_len = (uint64_t)&trampoline_end - (uint64_t)&trampoline_start;
    for (int i = 0; i < (trampoline_len / 4096) + 12; i++)
    {
        virt_map(0x1000 + (i * 4096), 0x1000 + (i * 4096), 0x1 | 0x2 | 0x4);
    }
    virt_map(0x4000, 0x4000, 0x1 | 0x2 | 0x4);
    virt_map(0x5000, 0x5000, 0x1 | 0x2 | 0x4);
    com_write_reg("trampoline lenght = ", trampoline_len);
    uint64_t end_addr = 0x4000;
    end_addr /= 4096;
    end_addr *= 4096;
    virt_map((uint64_t)end_addr, end_addr, 0x1 | 0x2 | 0x4);
    //   uint64_t *d = (uint64_t *)TRAMPOLINE_PAGING_ADDR;
    //   *d = ((uint64_t)pl4_table);
    virt_map(0x0, 0x0, 0x1 | 0x2 | 0x4);
    POKE(get_mem_addr(end_addr)) = ((uint64_t)pl4_table);
    POKE((end_addr)) = ((uint64_t)pl4_table);
    POKE((0x500)) = ((((uint64_t)pl4_table)));
    POKE((0x540)) = ((((uint64_t)pl4_table)));

    virt_map(get_rmem_addr((uint64_t)pl4_table), get_rmem_addr((uint64_t)pl4_table), 0x1 | 0x2 | 0x4);
    asm volatile(" \n"
                 "sgdt [0x580]\n"
                 "sidt [0x590]\n");
    POKE((0x520)) = ((((uint64_t)&cpuupstart)));
    uint64_t saddress = 0x8000;
    saddress /= 4096;
    saddress *= 4096;
    virt_map(saddress, saddress, 0x1 | 0x2 | 0x4);
    saddress += 4096;
    virt_map(saddress, saddress, 0x1 | 0x2 | 0x4);
    memset((void *)(saddress - 4096), 0, 4096);

    com_write_reg("stack raddr", saddress);
    com_write_reg("paging raddr ", ((uint64_t)&pl4_table));

    memcpy((void *)0x1000, &trampoline_start, trampoline_len);
    set_paging_dir(get_rmem_addr((uint64_t)pl4_table));
    com_write_reg("pre init cpu id : ", id);
    apic::the()->preinit_processor(apic);

    for (uint64_t i = 0; i < 1000; i++)
    {

        com_write_reg(" waiting ", i);
    }
    apic::the()->init_processor(apic, 0x1000);
    while (true)
    {
    }
    PIT::the()->Pwait(1000);
    while (SMPloaded != true)
    {
        PIT::the()->Pwait(1000);
    }
    SMPloaded = false;
}
smp *smp::the()
{
    return &main_smp;
}
