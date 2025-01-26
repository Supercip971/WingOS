
#include "kernel/generic/scheduler.hpp"

#include "libcore/ds/linked_list.hpp"

#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/lock/rwlock.hpp"

using SchedTasksList = core::Vec<kernel::Task *>;
SchedTasksList scheduler_list;
core::Vec<kernel::Task *> scheduler_idles;
core::RWLock scheduler_lock;
size_t running_cpu_count = 0;

bool is_scheduler_running = false;

size_t tick = 0;
namespace kernel
{
void idle()
{
    while (true)
    {
        asm volatile("hlt");
    }
}

core::Result<void> scheduler_init_idle_task()
{

    for (size_t i = 0; i < Cpu::count(); i++)
    {
        auto task = Task::task_create();
        if (task.is_error())
        {
            return "unable to create idle task";
        }
        auto t = task.unwrap();

        CpuContextLaunch launch = {};
        launch.entry = (void *)idle;
        launch.user = false;

        t->initialize(launch);
        scheduler_idles.push(t);
    }

    return {};
}

core::Result<void> scheduler_init([[maybe_unused]] int cpu_count)
{
    running_cpu_count = cpu_count;
    scheduler_lock.write_acquire();
    scheduler_list = SchedTasksList();

    scheduler_init_idle_task();
    scheduler_lock.write_release();
    return {};
}

core::Result<void> scheduler_start()
{
    return {};
}

core::Result<void> scheduler_tick()
{
    tick++;
    return {};
}

core::Result<kernel::Task *> next_task_select(CoreId core)
{

    if (scheduler_list.len() < running_cpu_count)
    {
        if (scheduler_list.len() < (size_t)core)
        {
            return scheduler_idles[core];
        }

        return scheduler_list[core];
    }

    return scheduler_list[(tick + core * 15) % scheduler_list.len()];
}

core::Result<void> task_run(TUID task_id, CoreId core)
{
    (void)core;
    auto task = Task::by_id(task_id);
    if (task.is_error())
    {
        return "unable to find task";
    }

    auto t = task.unwrap();

    scheduler_list.push(t);
    t->state(TaskState::TASK_RUNNING);

    return {};
}

core::Result<Task *> schedule(Task *current, void *state, CoreId core)
{

    core::lock_scope_reader$(scheduler_lock);
    if (current != nullptr)
    {
        current->cpu_context()->save_in(state);
    }
    auto next = try$(next_task_select(core));

    Cpu::current()->currentTask(next);

    next->cpu_context()->load_to(state);
    return next;
}
} // namespace kernel