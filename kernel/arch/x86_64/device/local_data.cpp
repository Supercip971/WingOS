#include <arch.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <logging.h>
#include <smp.h>
#include <sse.h>
cpu procData[smp::max_cpu];

ASM_FUNCTION void asm_sse_save(uintptr_t addr);
ASM_FUNCTION void asm_sse_load(uintptr_t addr);
void set_current_data(cpu *dat)
{

    dat->current_processor_id = apic::the()->get_current_processor_id();
    x86_wrmsr(LOCAL_DATA_DMSR, (uintptr_t)dat);
}
void cpu::load_sse(uint64_t *data)
{
    load_sse_context(data);
}
void cpu::save_sse(uint64_t *data)
{

    save_sse_context(data);
}
