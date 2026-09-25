#pragma once
#include <stdint.h>

#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/signal_asset.hpp"

#include "kernel/generic/asset.hpp"
#include "kernel/generic/space.hpp"
#include "libcore/result.hpp"

namespace kernel
{
void signal_trigger(AssetRef<SignalEndpoint> &endpoint);

fc::Result<AssetRef<SignalAttached>> signal_await(AssetRef<SignalEndpoint> &endpoint, AssetRef<AssetTask> &task, bool async, AssetRef<Space> &attached);

fc::Result<void> internal_signal_register(AssetRef<Space> root_space);

AssetRef<SignalEndpoint> signal_interrupt_query(int internal_id);
} // namespace kernel
