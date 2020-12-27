#pragma once
#include <64bit.h>
#include <arch.h>
#include <backtrace.h>
#include <device/apic.h>
#include <gdt.h>
#include <int_value.h>
#include <interrupt.h>
#include <process.h>
#include <smp.h>
#include <virtual.h>
#define LOCAL_DATA_DMSR 0xC0000100
class cpu
{
public:
    void *me;
    uint64_t stack_base;
    uint64_t current_processor_id;
    idtr cidt;
    gdtr cgdt;
    tss ctss;
    gdt_descriptor gdt_descriptors[64];

    uint8_t stack_data[8192] __attribute__((aligned(4096)));
    uint8_t stack_data_interrupt[8192] __attribute__((aligned(4096)));
    process *current_process;
    uint64_t lapic_id;
    main_page_table *page_table;

    uint64_t fpu_data[128] __attribute__((aligned(16)));
    void load_sse(uint64_t *data);
    void save_sse(uint64_t *data);
    backtrace local_backtrace;
};
void set_current_data(cpu *dat);
//local_data *get_current_data();
//local_data *get_current_data(int id);
extern cpu procData[smp::max_cpu];

inline cpu *get_current_cpu()
{
    uint64_t cc = 0;
    asm volatile("mov %0, fs \n"
                 : "=r"(cc));

    return &procData[cc];
}
inline cpu *get_current_cpu(int id)
{

    return &procData[id];
}
