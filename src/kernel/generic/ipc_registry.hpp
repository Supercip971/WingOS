#pragma once

#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/ipc_asset.hpp"

#include "libcore/result.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

namespace kernel
{

IpcServerHandle publish_server(AssetRef<kernel::IpcEndpoint> &endpoint, bool is_root);
void unregister_server(IpcServerHandle handle, AssetHandle space_handle);
uint64_t get_next_ipc_server_handle();

} // namespace kernel

fc::Result<AssetRef<kernel::IpcEndpoint>> query_server(IpcServerHandle handle);
