#pragma once
#include <stdint.h>

#include "arch/x86_64/context.hpp"

#include "kernel/generic/context.hpp"
#include "kernel/generic/paging.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"

namespace kernel
{
using TUID = uint64_t;

enum class TaskState
{
    TASK_NONE,
    TASK_EMBRYO,
    TASK_IDLE,
    TASK_RUNNING,
    TASK_DELETABLE,
    TASK_ZOMBIE,
};

// re-using the same keywords as Linux
enum TaskPriority : uint32_t
{
    TASK_PRIORITY_IDLE,
    TASK_PRIORITY_NORMAL,
    TASK_PRIORITY_REALTIME,
    TASK_PRIORITY_DEADLINE,
    TASK_PRIORITY_COUNT,
};

static constexpr size_t TASK_QUEUE_COUNT = 128;

class Task
{
    TUID _uid = 0;
    TaskState _state = TaskState::TASK_NONE;
    CpuContext *_cpu_context;

public:
    TaskState state() const { return _state; }
    void state(TaskState state) { _state = state; }

    Task() = default;
    Task(TUID uid) : _uid(uid) {};


    core::Result<void> _initialize(CpuContextLaunch params, VmmSpace* target_vspace);


    virtual ~Task() = default;

    TUID uid() const { return _uid; }

    // task current is obtained through cpu::current()->task()
    // static Task *current();

    static Task *by_id_unsafe(TUID uid);

    static core::Result<Task *> by_id(TUID uid);

    static core::Result<Task *> task_create();

    VmmSpace &vmm_space() { return *_cpu_context->_vmm_space; };

    void user_stack_addr(uintptr_t addr) { _cpu_context->use_stack_addr(addr); }

    CpuContext *cpu_context() { return _cpu_context; }
};

} // namespace kernel