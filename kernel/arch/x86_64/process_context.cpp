#include <device/local_data.h>
#include <gdt.h>
#include <process.h>
#include <process_context.h>
#include <smp.h>
#include <sse.h>
#include <utility.h>
#include <utils/liballoc.h>
#include <utils/lock.h>
#include <virtual.h>
extern int process_locked;
extern bool cpu_wait;
uint8_t proc_last_selected_cpu = 0;

void send_switch_process_to_all_cpu()
{
    if (apic::the()->get_current_processor_id() == 0)
    {
        for (uint32_t i = 0; i <= smp::the()->processor_count; i++)
        {
            if (i != apic::the()->get_current_processor_id())
            {
                apic::the()->send_ipi(i, 100);
            }
        }
    }
}
void add_thread_map(process *p, uintptr_t from, uintptr_t to, uint64_t length)
{
    for (uint64_t i = 0; i < length; i++)
    {
        map_page((uint64_t *)p->get_arch_info().page_directory, from + i * PAGE_SIZE, to + i * PAGE_SIZE, PAGE_TABLE_FLAGS);
    }
}
void task_update_switch(process *next)
{
    tss_set_rsp0((uint64_t)next->get_arch_info().stack + PROCESS_STACK_SIZE);

    get_current_cpu()->page_table = (uint64_t *)next->get_arch_info().page_directory;
    update_paging();
}

uintptr_t switch_context(InterruptStackFrame *current_Isf, process *next)
{

    if (process_locked != 0)
    {
        return (uintptr_t)current_Isf; // early return
    }

    if (next == NULL)
    {
        if (cpu_wait)
        {
            cpu_wait = false;
        }

        return (uintptr_t)current_Isf; // early return
    }

    if (process::current() != NULL)
    {
        process::current()->set_state(PROCESS_WAITING);
        process::current()->get_arch_info().rsp = (uint64_t)current_Isf;

        get_current_cpu()->save_sse(process::current()->get_arch_info().sse_context);
    }
    next->set_state(process_state::PROCESS_RUNNING);
    process::set_current(next);
    get_current_cpu()->load_sse(process::current()->get_arch_info().sse_context);
    task_update_switch(next);

    if (cpu_wait)
    {
        cpu_wait = false;
    }
    return next->get_arch_info().rsp;
}
void init_process_stackframe(process *pro, func entry_point)
{
    memzero(pro->get_arch_info().stack, PROCESS_STACK_SIZE);
    pro->get_arch_info().rsp =
        ((uint64_t)pro->get_arch_info().stack) + PROCESS_STACK_SIZE;

    uint64_t *rsp = (uint64_t *)pro->get_arch_info().rsp;

    InterruptStackFrame *ISF = (InterruptStackFrame *)(rsp - (sizeof(InterruptStackFrame)));

    ISF->rip = (uint64_t)entry_point;
    ISF->ss = gdt_selector::KERNEL_DATA;
    ISF->cs = gdt_selector::KERNEL_CODE;
    ISF->rflags = 0x286;
    ISF->rsp = (uint64_t)ISF;
    pro->get_arch_info().rsp = (uint64_t)ISF;
}

uint64_t interpret_cpu_request(uint64_t cpu)
{
    if (cpu == CURRENT_CPU)
    {
        return apic::the()->get_current_processor_id();
    }
    else if (cpu == AUTO_SELECT_CPU)
    {
        proc_last_selected_cpu++;

        if (proc_last_selected_cpu > smp::the()->processor_count)
        {
            proc_last_selected_cpu = 0;
        }

        cpu = proc_last_selected_cpu;
    }
    return cpu;
}

void init_process_paging(process *pro, bool is_user)
{
    if (is_user)
    {
        pro->get_arch_info().page_directory = (uint64_t)new_vmm_page_dir();
    }
    else
    {
        pro->get_arch_info().page_directory = (uint64_t)get_current_cpu()->page_table;
    }
}

void init_process_arch_ext(process *pro)
{

    get_current_cpu()->save_sse(pro->get_arch_info().sse_context);
}
