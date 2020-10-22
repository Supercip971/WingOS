#include <arch/arch.h>
#include <arch/smp.h>
#include <arch/sse.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <logging.h>
cpu procData[smp::max_cpu];

extern "C" void asm_sse_save(uint64_t addr);
extern "C" void asm_sse_load(uint64_t addr);
void set_current_data(cpu *dat)
{

    dat->me = dat;
    dat->current_processor_id = apic::the()->get_current_processor_id();
    x86_wrmsr(LOCAL_DATA_DMSR, (uint64_t)dat);
}
void cpu::load_sse(uint64_t *data)
{
    load_sse_context(data);
}
void cpu::save_sse(uint64_t *data)
{

    save_sse_context(data);
}
