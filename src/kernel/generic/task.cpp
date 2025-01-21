#include <stdint.h>

#include "hw/mem/addr_space.hpp"
#include <libcore/ds/linked_list.hpp>

#include "kernel/generic/kernel.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/lock/rwlock.hpp"
#include "libcore/result.hpp"

static core::LinkedList<kernel::Task> task_list = {};
static kernel::TUID next_uid = 1;
static core::RWLock _task_lock = {};

kernel::Task *_task_allocate()
{
    core::lock_scope_writer$(_task_lock);

    kernel::Task task(next_uid++);
    task.state(kernel::TaskState::TASK_EMBRYO);

    return task_list.push(task);
}

kernel::Task *kernel::Task::by_id_unsafe(kernel::TUID uid)
{
    core::lock_scope_reader$(_task_lock);

    for (auto b = task_list.begin(); b != task_list.end(); ++b)
    {
        if ((*b)._uid == uid)
        {
            return b._ptr->data.as_ptr();
        }
    }

    return nullptr;
}

core::Result<kernel::Task *> kernel::Task::by_id(kernel::TUID uid)
{
    auto task = kernel::Task::by_id_unsafe(uid);
    if (task == nullptr)
    {
        return "unable to find task";
    }

    return task;
}

core::Result<kernel::Task *> kernel::Task::task_create()
{
    kernel::Task *task = _task_allocate();
    if (task == nullptr)
    {
        return "unable to allocate task";
    }

    task->_cpu_context = try$(kernel::CpuContext::create_empty());
    task->_vmm_space = try$(VmmSpace::create(false));

    return task;
}

core::Result<void> kernel::Task::initialize(CpuContextLaunch params)
{
    auto task = this;

    task->_cpu_context->prepare(params);

    task->_state = kernel::TaskState::TASK_IDLE;

    if (params.user)
    {
        VirtRange userspace_stack = VirtRange{
            userspace_stack_base - userspace_stack_size,
            userspace_stack_base};

        PhysRange phys_userspace_stack = PhysRange(
            toPhys(addrOf(task->_cpu_context->stack_ptr)),
            toPhys(addrOf(task->_cpu_context->stack_top)));

        task->vmm_space().map(userspace_stack, phys_userspace_stack,
                              PageFlags()
                                  .user(true)
                                  .executable(false)
                                  .present(true)
                                  .writeable(true));

        task->_cpu_context->use_stack_addr(userspace_stack_base);
    }

    return {};
}

namespace kernel
{

}