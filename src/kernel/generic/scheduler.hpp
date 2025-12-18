
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "kernel/generic/cpu.hpp"
#include "libcore/logic.hpp"
#include "libcore/result.hpp"
#include <kernel/generic/blocker.hpp>

namespace kernel
{


using TUID = uint64_t;

static constexpr size_t TASK_QUEUE_COUNT = 128;
class Task;
struct SchedulerControlBlock
{
    bool is_idle = false;
    CoreId cpu_affinity = CpuCoreNone;
    CoreId old_cpu_affinity = CpuCoreNone;
    size_t total_cycles = 0;
    BlockEvent block_event = {};
    BlockEvent incoming_block_event = {};

    // note that running and sleeping are not counted in terms of ticks but rather are weighted
    long running = 0;
    long sleeping = 0;
    long priority = 0;

    SchedulerControlBlock() : priority(0) {}
    SchedulerControlBlock(size_t _priority) : priority(_priority) {}

    void tick()
    {
        total_cycles++;
    }

    long weight() const
    {
        return priority + running - sleeping;
    }

    long queue() const
    {
        return core::clamp(weight() + (long)TASK_QUEUE_COUNT / 2, 0, (long)TASK_QUEUE_COUNT - 1);
    }
};

core::Result<void> scheduler_init(int cpu_count);

core::Result<void> scheduler_start();

core::Result<Task *> next_task_select(CoreId core);

core::Result<void> scheduler_tick();

core::Result<void> task_run(TUID task_id, CoreId core = 0);

core::Result<Task *> schedule(Task *current, void *state, CoreId core, bool soft = false);
core::Result<void> dump_current_running_task();


core::Result<void> dump_all_current_running_tasks();

core::Result<void> block_current_task(BlockEvent event);

core::Result<void> resolve_blocked_tasks();

void schedule_if_task_blocked();

} // namespace kernel
// arch implemented
void trigger_reschedule(CoreId cpu);
void trigger_reschedule_unblocked(CoreId cpu);


void reschedule_self();