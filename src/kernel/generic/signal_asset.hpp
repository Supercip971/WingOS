#pragma once
#include <atomic>
#include <stdint.h>

#include "kernel/generic/asset_types.hpp"

#include "kernel/generic/asset.hpp"
#include "libcore/ds/vec.hpp"

struct Space;

namespace kernel
{

class SignalEndpoint : public Asset
{

public:
    std::atomic<uint64_t> _counter = 0;
    fc::Vec<AssetRef<AssetTask>> _awaiters;

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

} // namespace kernel
