#pragma once
#include <stdint.h>

#include "arch/x86_64/context.hpp"

#include "kernel/generic/context.hpp"
#include "kernel/generic/paging.hpp"
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

class Task
{
    TUID _uid = 0;
    TaskState _state = TaskState::TASK_NONE;
    VmmSpace _vmm_space;
    CpuContext *_cpu_context;

public:
    TaskState state() const { return _state; }
    void state(TaskState state) { _state = state; }

    Task() = default;
    Task(TUID uid) : _uid(uid) {};

    core::Result<void> initialize(CpuContextLaunch params);

    virtual ~Task() = default;

    TUID uid() const { return _uid; }

    // task current is obtained through cpu::current()->task()
    // static Task *current();

    static Task *by_id_unsafe(TUID uid);

    static core::Result<Task *> by_id(TUID uid);

    static core::Result<Task *> task_create();

    VmmSpace &vmm_space() { return _vmm_space; };

    void user_stack_addr(uintptr_t addr) { _cpu_context->use_stack_addr(addr); }
};

} // namespace kernel