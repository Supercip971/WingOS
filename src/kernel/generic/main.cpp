

#include <kernel/generic/kernel.hpp>
#include <libcore/fmt/log.hpp>
#include <stdlib.h>

#include "kernel/generic/pmm.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/fmt/flags.hpp"

void fun1()
{
    while (true)
    {
        log::log$("fun1");
        asm volatile("hlt");
    }
}

void fun2()
{
    while (true)
    {
        log::log$("fun2");
        asm volatile("hlt");
    }
}

void fun3()
{
    while (true)
    {
        log::log$("fun3");
        asm volatile("hlt");
    }
}

void kernel_entry(const mcx::MachineContext *context)
{

    (void)context;

    log::log$("started kernel");

    kernel::scheduler_init(1).assert();

    kernel::Task *task1 = kernel::Task::task_create().unwrap();
    kernel::Task *task2 = kernel::Task::task_create().unwrap();
    kernel::Task *task3 = kernel::Task::task_create().unwrap();

    task1->initialize({.entry = (void *)fun1, .user = false}).assert();
    task2->initialize({.entry = (void *)fun2, .user = false}).assert();
    task3->initialize({.entry = (void *)fun3, .user = false}).assert();

    log::log$("task1: {}", task1->uid());
    log::log$("task2: {}", task2->uid());
    log::log$("task3: {}", task3->uid());

    kernel::task_run(task1->uid()).assert();
    kernel::task_run(task2->uid()).assert();
    kernel::task_run(task3->uid()).assert();

    asm volatile("sti");

    while (true)
    {
        asm volatile("hlt");
    }
}