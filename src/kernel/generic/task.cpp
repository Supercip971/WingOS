#include <atomic>
#include <libcore/fmt/log.hpp>
#include <stdint.h>

#include "hw/mem/addr_space.hpp"
#include <libcore/ds/linked_list.hpp>

#include "kernel/generic/context.hpp"
#include "kernel/generic/kernel.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/lock/rwlock.hpp"
#include "libcore/result.hpp"

fc::LinkedList<kernel::Task *> task_list = {};
bool loaded = false;
std::atomic<kernel::TUID> next_uid = 1;
fc::RWLock _task_lock = {};

kernel::Task *kernel::Task::_task_allocate()
{
    if (!loaded)
    {
        task_list = {};
        next_uid = 1;
        _task_lock.full_reset();

        loaded = true;
    }
    fc::lock_scope_writer$(_task_lock);

    kernel::Task *task = new kernel::Task();

    task->uid(next_uid++);
    task->state(kernel::TaskState::TASK_EMBRYO);

    task->ref_count++;
    task_list.push(task);

    return task;
}

kernel::Task *kernel::Task::by_id_unsafe(kernel::TUID uid)
{
    fc::lock_scope_reader$(_task_lock);

    for (auto b = task_list.begin(); b != task_list.end(); ++b)
    {
        if ((*b)->_uid == uid)
        {
            return *b;
        }
    }

    return nullptr;
}

fc::Result<kernel::Task *> kernel::Task::by_id(kernel::TUID uid)
{
    auto task = kernel::Task::by_id_unsafe(uid);
    if (task == nullptr)
    {
        fmt::warn$("unable to find task {}", uid);
        return "unable to find task";
    }

    return task;
}

fc::Result<kernel::Task *> kernel::Task::task_create()
{
    kernel::Task *task = kernel::Task::_task_allocate();
    if (task == nullptr)
    {
        return ("unable to allocate task");
    }

    task->_cpu_context = try$(kernel::CpuContext::create_empty());

    fmt::log$("created task with id: {}", task->uid());
    return (task);
}

fc::Result<void> kernel::Task::_initialize(CpuContextLaunch params, VmmSpace *target_vspace)
{
    try$(_cpu_context->prepare(params));

    _state = kernel::TaskState::TASK_IDLE;

    _cpu_context->_vmm_space = target_vspace;

    if (params.user)
    {
        VirtRange userspace_stack = VirtRange{
            userspace_stack_base - userspace_stack_size,
            userspace_stack_base};

        PhysRange phys_userspace_stack = PhysRange(
            toPhys(addrOf(_cpu_context->stack_ptr)),
            toPhys(addrOf(_cpu_context->stack_top)));

        vmm_space().map(userspace_stack, phys_userspace_stack,
                        PageFlags()
                            .user(true)
                            .executable(false)
                            .present(true)
                            .writeable(true));

        _cpu_context->use_stack_addr(userspace_stack_base);
    }

    return {};
}

namespace kernel
{

}
