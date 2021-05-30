#include <device/local_data.h>
#include <gdt.h>
#include <mem/virtual.h>
#include <proc/process.h>
#include <process_context.h>
#include <smp.h>
#include <sse.h>
#include <utility.h>
#include <utils/lock.h>
#include <utils/memory/liballoc.h>
extern int process_locked;
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
        map_page(p->get_arch_info()->page_directory, from + i * PAGE_SIZE, to + i * PAGE_SIZE, true, true);
    }
}
void task_update_switch(process *next)
{
    tss_set_rsp0((uint64_t)next->get_arch_info()->stack + PROCESS_STACK_SIZE);

    get_current_cpu()->cpu_page_table = next->get_arch_info()->page_directory;
    set_paging_dir(get_physical_addr((uintptr_t)next->get_arch_info()->page_directory));
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

        return (uintptr_t)current_Isf; // early return
    }

    if (process::current() != NULL)
    {
        if (process::current()->get_state() != PROCESS_SHOULD_BE_DEAD)
        {

            process::current()->set_state(PROCESS_WAITING);
            process::current()->get_arch_info()->rsp = (uint64_t)current_Isf;
            process::current()->get_arch_info()->status = *current_Isf;
            get_current_cpu()->save_sse(process::current()->get_arch_info()->sse_context);
        }
    }

    next->set_state(process_state::PROCESS_RUNNING);
    process::set_current(next);
    *current_Isf = process::current()->get_arch_info()->status;

    get_current_cpu()->load_sse(process::current()->get_arch_info()->sse_context);
    task_update_switch(next);

    return next->get_arch_info()->rsp;
}
void init_process_stackframe(process *pro, func entry_point, int argc, char **argv)
{
    InterruptStackFrame *ISF;
    if (pro->is_user())
    {

        arch_process_data *target_arch = (pro->get_arch_info());
        void *physical_mem = (void *)get_mem_addr(pmm_alloc(PROCESS_STACK_SIZE / PAGE_SIZE + 1));

        void *phys_rsp = (uint8_t *)physical_mem + PROCESS_STACK_SIZE;
        void *phys_stack = physical_mem;
        memzero(phys_stack, PROCESS_STACK_SIZE + PAGE_SIZE);

        void *virtual_rsp = (void *)(USR_STK_OFFSET);

        void *virtual_stack = (void *)(USR_STK_OFFSET - PROCESS_STACK_SIZE);

        target_arch->stack = (uint8_t *)(virtual_stack);
        target_arch->rsp = (uintptr_t)(virtual_rsp);

        add_thread_map(pro, get_rmem_addr((uintptr_t)phys_stack), (uintptr_t)(virtual_stack), PROCESS_STACK_SIZE / PAGE_SIZE + 1);

        ISF = (InterruptStackFrame *)(((uint8_t *)phys_rsp) - (sizeof(InterruptStackFrame)));
        ISF->rsp = (uint64_t)virtual_rsp - (sizeof(InterruptStackFrame));
        target_arch->rsp = (uintptr_t)ISF;
    }
    else
    {
        pro->get_arch_info()->stack = (uint8_t *)get_mem_addr(pmm_alloc(PROCESS_STACK_SIZE / PAGE_SIZE + 1));
        pro->get_arch_info()->rsp =
            ((uint64_t)pro->get_arch_info()->stack) + PROCESS_STACK_SIZE;
        memzero(pro->get_arch_info()->stack, PROCESS_STACK_SIZE + 1);

        ISF = (InterruptStackFrame *)(pro->get_arch_info()->rsp - (sizeof(InterruptStackFrame)) - 8);

        ISF->rsp = (uint64_t)ISF;
        pro->get_arch_info()->rsp = (uint64_t)ISF;
    }

    pro->get_arch_info()->process_stack_size = PROCESS_STACK_SIZE;

    ISF->rflags = 0x286;
    ISF->rip = (uint64_t)entry_point;
    ISF->rbp = ISF->rsp - sizeof(InterruptStackFrame);
    ISF->rdi = argc;
    ISF->rsi = (uint64_t)argv;

    if (pro->is_user())
    {

        add_thread_map(pro, (uintptr_t)get_physical_addr((uintptr_t)argv), (uintptr_t)argv, 1);
        for (int i = 0; i < argc; i++)
        {
            add_thread_map(pro, (uintptr_t)get_physical_addr((uintptr_t)argv[i]), (uintptr_t)argv[i], 1);
        }

        ISF->ss = gdt_selector::USER_DATA;
        ISF->cs = gdt_selector::USER_CODE;
    }
    else
    {

        ISF->ss = gdt_selector::KERNEL_DATA;
        ISF->cs = gdt_selector::KERNEL_CODE;
    }
    pro->get_arch_info()->status = *ISF;

    log("proc", LOG_INFO, "process stack {}", pro->get_arch_info()->rsp);
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
        pro->get_arch_info()->page_directory = new_vmm_page_dir();
    }
    else
    {
        pro->get_arch_info()->page_directory = get_current_cpu()->cpu_page_table;
    }
}

void init_process_arch_ext(process *pro)
{

    get_current_cpu()->save_sse(pro->get_arch_info()->sse_context);
}

void *realloc_process_stack(InterruptStackFrame *current_isf, process *target)
{
    arch_process_data *target_arch = target->get_arch_info();
    target_arch->process_stack_size += PAGE_SIZE * 2;
    target_arch->stack -= PAGE_SIZE * 2;
    void *target_new_stack = pmm_alloc(2);
    add_thread_map(target, (uintptr_t)target_new_stack, (uintptr_t)target_arch->stack, 2);

    return nullptr;
}