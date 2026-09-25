#pragma once
#include <atomic>
#include <stdint.h>

#include "kernel/generic/asset_types.hpp"

#include "kernel/generic/asset.hpp"
#include "libcore/ds/vec.hpp"

struct Space;

namespace kernel
{

enum WaiterKind
{
    WAITER_KIND_ASYNC,
    WAITER_KIND_SYNC,
};

struct Waiter
{
    AssetRef<AssetTask> task;
    WaiterKind kind;
    AssetRef<> attachement;
};

class SignalEndpoint : public Asset
{

public:
    std::atomic<uint64_t> _counter = 0;
    fc::Vec<Waiter> _awaiters;

    SignalEndpoint() : Asset(AssetKind::OBJECT_KIND_SIGNAL_ENDPOINT) {}

    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_SIGNAL_ENDPOINT;

    auto counter()
    {
        this->lock.lock();
        auto v = _counter.load();
        this->lock.release();
        return v;
    }

    void counter_increment() { _counter.fetch_add(1); }

    virtual ~SignalEndpoint() = default;
};

class SignalAttached : public Asset
{

public:
    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_SIGNAL_ATTACHED;
    AssetRef<SignalEndpoint> endpoint;

    SignalAttached(AssetRef<SignalEndpoint> _endpoint) : Asset(AssetKind::OBJECT_KIND_SIGNAL_ATTACHED), endpoint(_endpoint) {}

    void detach()
    {
        long id = 0;
        endpoint.asset->lock.lock();
        for (long i = 0; i < (long)endpoint->_awaiters.len(); ++i)
        {
            if (endpoint->_awaiters[i].attachement.asset == this)
            {
                id = i;
                break;
            }
        }
        auto dthis = endpoint.asset->_awaiters.pop(id);
        endpoint.asset->lock.release();
        //
    }
};
} // namespace kernel
