#include <stdint.h>

#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/signal_asset.hpp"

#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/space.hpp"
#include "libcore/ds/umap.hpp"
#include "signal.hpp"

static fc::UMap<int, AssetRef<kernel::SignalEndpoint>> _internal_signal_map = {};

void kernel::signal_trigger(AssetRef<SignalEndpoint> &endpoint)
{

    // early return
    endpoint->counter_increment();
    if (endpoint->_awaiters.len() == 0)
    {
        return;
    }
    endpoint.lock();

    size_t index = 0;
    while (index < endpoint->_awaiters.len())
    {
        auto &elt = endpoint->_awaiters[index];
        if (!elt.task.asset)
        {
            index++;
            continue;
        }
        elt.task.asset->signaled = true;
        elt.task.asset->signaled_by = elt.attachement.get_handle();
        elt.task->sched().unblock();

        if (elt.kind == WaiterKind::WAITER_KIND_SYNC)
        {
            endpoint->_awaiters.pop(index);
            continue;
        }
        index++;
    }

    endpoint.unlock();
    resolve_blocked_tasks();
}

/*
void kernel::signal_trigger_from_irq(AssetRef<SignalEndpoint> &endpoint)
{
    if (!endpoint->lock.retry_try_lock())
    {
        endpoint->counter_increment();
        for (auto &awaiter : endpoint->_awaiters)
        {
            if (awaiter.asset)
                awaiter->sched().unblock();
        }
        // don't clear
        resolve_blocked_tasks();
        return;
    }

    endpoint->counter_increment();
    for (auto &awaiter : endpoint->_awaiters)
    {
        if (awaiter.asset)
            awaiter->sched().unblock();
    }
    endpoint->_awaiters.clear();
    endpoint->lock.release();
    resolve_blocked_tasks();
    }*/

fc::Result<AssetRef<kernel::SignalAttached>> kernel::signal_await(AssetRef<SignalEndpoint> &endpoint, AssetRef<AssetTask> &task, bool async, AssetRef<Space> &host_space)
{

    auto attached = try$(host_space->create_attached_signal(endpoint));

    Cpu::current()->interrupt_hold();
    while (!endpoint->lock.retry_try_lock())
    {
        Cpu::current()->interrupt_release();
        arch::pause();
        Cpu::current()->interrupt_hold();
    }
    auto wait_id = endpoint->_counter.load();

    endpoint->_awaiters.push({task, async ? WAITER_KIND_ASYNC : WAITER_KIND_SYNC, attached});

    endpoint->lock.release();
    Cpu::current()->interrupt_release();

    if (async)
        return attached;

    while (endpoint->_counter.load() == wait_id)
    {
        task.lock();
        task->sched().block();

        resolve_blocked_tasks();

        task.unlock();

        block_current_task();
    }

    host_space->asset_release(attached);
    return {};
}

fc::Result<void> kernel::internal_signal_register(AssetRef<Space> root_space)
{
    for (size_t i = 0; i < 32; i++)
    {
        _internal_signal_map.insert(i, try$(root_space->create_signal_endpoint()));
    }

    return {};
}

AssetRef<kernel::SignalEndpoint> kernel::signal_interrupt_query(int internal_id)
{
    if (!_internal_signal_map.has(internal_id))
        return {};
    return _internal_signal_map[internal_id];
}
