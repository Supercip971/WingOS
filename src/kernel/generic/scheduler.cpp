
#include "kernel/generic/scheduler.hpp"

#include "kernel/generic/cpu_tree.hpp"
#include "libcore/ds/linked_list.hpp"

#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/ds/array.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/rwlock.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"

using TaskQueue = core::LinkedList<kernel::SchedulerEntity>;

core::Vec<kernel::Task *> scheduler_idles;
core::Vec<kernel::SchedulerEntity> cpu_runned;
core::Array<TaskQueue, kernel::TASK_QUEUE_COUNT> task_queues;

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
        auto sched_entity = SchedulerEntity(t, 0);
        sched_entity.is_idle = true;
        cpu_runned.push(sched_entity);
    }

    return {};
}

core::Result<void> scheduler_init([[maybe_unused]] int cpu_count)
{
    running_cpu_count = cpu_count;
    scheduler_lock.write_acquire();

    for (size_t i = 0; i < kernel::TASK_QUEUE_COUNT; i++)
    {
        task_queues[i] = TaskQueue();
    }

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
    core::lock_scope_reader$(scheduler_lock);

    return cpu_runned[core].task;
}

static inline void add_entity_to_queue(SchedulerEntity task)
{

    if (task.is_idle)
    {
        return;
    }
    //    log::log$("adding task {} to queue {}", task.task->uid(), task.queue());
    task_queues[task.queue()].push(task);
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

    SchedulerEntity entity(t, 0);
    t->state(TaskState::TASK_RUNNING);

    add_entity_to_queue(entity);
    return {};
}

// FIXME: store it as a global variable instead of recalculating it
static inline size_t scheduled_task_count()
{
    size_t count = 0;
    for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
    {
        count += task_queues[i].count();
    }

    for (size_t i = 0; i < running_cpu_count; i++)
    {
        if (!cpu_runned[i].is_idle)
        {
            count++;
        }
    }

    return count;
}

static core::Result<int> query_nearest_task(size_t queue_id, CoreId core, bool consider_siblings = false)
{
    // For tasks in the given queue, choose the one whose CPU affinity is either unset,
    // already matching the given CPU, or whose affinity is “close” (i.e. sibling) to the given CPU.

    auto &queue = task_queues[queue_id];
    for (auto task = queue.begin(); task != queue.end(); ++task)
    {
        if ((*task).cpu_affinity == CpuCoreNone)
        {
            (*task).cpu_affinity = core;
            // first time runned with no affinity
            return queue_id;
        }

        if ((*task).cpu_affinity == core)
        {
            return queue_id;
        }
        if (consider_siblings && CpuTreeNode::root()->are_they_sibling((*task).cpu_affinity, core))
        {
            return queue_id;
        }
    }

    return {-1};
}

static long summed_weights()
{
    long sum = 0;
    for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
    {
        for (auto task = task_queues[i].begin(); task != task_queues[i].end(); ++task)
        {
            sum += (*task).weight();
        }
    }

    // for(size_t i = 0; i < running_cpu_count; i++)
    //{
    //     if(!cpu_runned[i].is_idle)
    //     {
    //         sum += cpu_runned[i].weight();
    //     }
    // }

    return sum;
}

static void update_runned_tasks()
{
    // actually quick because of linked list
    // we shift everything from a queue to the previous one
    // to avoid the need of sorting
    for (size_t i = 0; i < TASK_QUEUE_COUNT - 1; i++)
    {
        if (i == 0)
        {
            task_queues[i] += core::move(task_queues[i + 1]);
        }
        else
        {
            task_queues[i] = core::move(task_queues[i + 1]);
        }
    }
    task_queues[TASK_QUEUE_COUNT - 1].release();

    // technically we should be able to do this in one iteration, and this is a bit of a waste
    // but for now, I prefer having readable code rather than optimized one
    // - say the guy who spend too much time optimizing a scheduler algorithm before having a userspace

    auto sum = summed_weights();
    auto count = scheduled_task_count();

    if (count != 0)
    {

        long avg_sleep_time = (sum + count - 1) / count; // integer ceil (TODO: check)
        for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
        {
            for (auto task = task_queues[i].begin(); task != task_queues[i].end(); ++task)
            {
                (*task).sleeping += avg_sleep_time;
            }
        }
    }

    for (size_t i = 0; i < running_cpu_count; i++)
    {
        if (cpu_runned[i].is_idle)
        {
            continue;
        }
        cpu_runned[i].running += 1;
        cpu_runned[i].old_cpu_affinity = i;
        add_entity_to_queue(cpu_runned[i]);

        cpu_runned[i].task = scheduler_idles[i];
        cpu_runned[i].is_idle = true;
    }
}

static core::Result<void> fix_sched_affinity()
{
    size_t attempt = 0;
    static core::Vec<CoreId> to_fix = {};

    // technically, should be allocated once
    for (size_t i = 0; i < running_cpu_count; i++)
    {
        to_fix.push(i);
    }

    const size_t max_attempt = 4 * running_cpu_count;

    while (attempt < max_attempt && to_fix.len() > 0)
    {
        CoreId current = to_fix.pop();
        auto &task_associated = cpu_runned[current];
        if (task_associated.is_idle)
        {
            continue;
        }

        auto old_cpu = task_associated.old_cpu_affinity;

        // can't fix a task that is already on a good affinity
        // or a task that is discovering the current cpu
        if (old_cpu == current || old_cpu == CpuCoreNone)
        {
            continue;
        }

        auto &stealer = cpu_runned[old_cpu];
        stealer.cpu_affinity = current;
        cpu_runned[current] = stealer;

        task_associated.cpu_affinity = old_cpu;
        cpu_runned[old_cpu] = task_associated;

        to_fix.push(old_cpu);
    }

    to_fix.clear();
    if (attempt >= 4 * running_cpu_count)
    {
        log::err$("unable to fix affinity, attempt limit reached");

        return "unable to fix affinity, attempt limit reached";
    }

    return {};
}

void run_task_queued(CoreId cpu, size_t queue_id, size_t queue_offset)
{
    auto task = task_queues[queue_id].remove(queue_offset);

    task.tick();
    task.cpu_affinity = cpu;
    task.is_idle = false;
    cpu_runned[cpu] = task;
}

core::Vec<size_t> _choosen;
core::Vec<size_t> _retried;
core::Result<void> schedule_all()
{

    auto *choosen_ptr = &_choosen;
    auto *retried_ptr = &_retried;
    auto c = scheduled_task_count();
    if (c == 0)
    {
        return {};
    }

    for (size_t i = 0; i < running_cpu_count; i++)
    {
        choosen_ptr->push(i);
    }

    for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
    {
        retried_ptr->clear();

        // first for the current priority,
        // match the processor with the task with the best affinity
        while (choosen_ptr->len() != 0)
        {
            auto cpu = choosen_ptr->pop();
            auto task = query_nearest_task(i, cpu, false);

            if (task.is_error())
            {
                retried_ptr->push(cpu);
                continue;
            }
            auto val = task.unwrap();
            if (val == -1)
            {
                retried_ptr->push(cpu);
                continue;
            }

            run_task_queued(cpu, val, 0);
        }

        // then if we still have task and cpus but with not matching affinity, we still
        // assign them (to avoid task stalling)
        while (retried_ptr->len() != 0 && task_queues[i].count() > 0)
        {
            auto cpu = retried_ptr->pop();
            run_task_queued(cpu, i, 0);
        }

        if (retried_ptr->len() == 0)
        {
            return {};
        }

        // we retry for task with a lower priority by swapping the retried task,

        core::swap(choosen_ptr, retried_ptr);
    }

    for (size_t i = 0; i < choosen_ptr->len(); i++)
    {
        auto cpu = (*choosen_ptr)[i];
        cpu_runned[cpu].is_idle = true;
        cpu_runned[cpu].task = scheduler_idles[cpu];
    }
    return {};
}

core::Result<void> reschedule_all()
{
    core::lock_scope_writer$(scheduler_lock);
    tick++;

    update_runned_tasks();
    try$(schedule_all());
    try$(fix_sched_affinity());
    return {};
}

core::Result<Task *> schedule(Task *current, void *state, CoreId core)
{
    if (core == 0)
    {
        auto v = reschedule_all();
        if (v.is_error())
        {
            log::err$("unable to reschedule all tasks");
            log::err$("we will continue to run the current ones");
            return {};
        }
    }

    core::lock_scope_reader$(scheduler_lock);

    auto next = try$(next_task_select(core));
    if (current != nullptr)
    {
        current->cpu_context()->save_in(state);

        
        // log::log$("scheduling task on core {}, {} -> {}", core, current->uid(), next->uid());
    }

    Cpu::current()->currentTask(next);

    next->cpu_context()->load_to(state);
    return next;
}
} // namespace kernel