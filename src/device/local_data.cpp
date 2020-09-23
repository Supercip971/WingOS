#include <arch/arch.h>
#include <arch/smp.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <loggging.h>
local_data procData[smp::max_cpu];

local_data *get_current_data()
{
    if (apic::the()->isloaded() == false)
    {
        log("local data", LOG_ERROR) << ("getting current data of cpu 0");
        return &procData[0];
    }
    else
    {
        return &procData[apic::the()->get_current_processor_id()];
    }
}
local_data *get_current_data(int id)
{

    return &procData[id];
}
void set_current_data(local_data *dat)
{

    dat->me = dat;
    dat->current_processor_id = apic::the()->get_current_processor_id();
    x86_wrmsr(LOCAL_DATA_DMSR, (uint64_t)dat);
}
