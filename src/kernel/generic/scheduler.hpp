
#pragma once 
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/result.hpp"

namespace kernel 
{
    core::Result<void> scheduler_init(int cpu_count);

    core::Result<void> scheduler_start();

    core::Result<Task*> next_task_select(CoreId core);

    core::Result<void> scheduler_tick();

    core::Result<void> task_run(TUID task_id, CoreId core=0);

    core::Result<Task*> schedule(Task* current, void* state,CoreId core);
}