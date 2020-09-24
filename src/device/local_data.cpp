#include <arch/arch.h>
#include <arch/smp.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <loggging.h>
local_data procData[smp::max_cpu];

void set_current_data(local_data *dat)
{

    dat->me = dat;
    dat->current_processor_id = apic::the()->get_current_processor_id();
    x86_wrmsr(LOCAL_DATA_DMSR, (uint64_t)dat);
}
