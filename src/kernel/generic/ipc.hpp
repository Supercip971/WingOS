#pragma once

#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/ipc_asset.hpp"

#include "libcore/result.hpp"
#include "space.hpp"

// Forward declarations to avoid pulling `space.hpp` (and its dependencies) into IPC.

namespace kernel
{

IpcEndpoint *create_ipc_endpoint();

fc::Result<AssetRef<IpcEndpointConnection>> ipc_connect(AssetRef<Space> &target_space, AssetRef<IpcEndpoint> &endpoint);

fc::Result<void> ipc_receive(AssetRef<Space> &space, AssetRef<AssetTask> &callee, AssetRef<IpcEndpoint> &endpoint, IpcMessage *target, uint64_t *ret_object_handle);

fc::Result<void> ipc_receive_async(AssetRef<Space> &space, AssetRef<IpcEndpoint> &endpoint, IpcMessage *target, uint64_t *ret_object_handle);

fc::Result<void> ipc_send(AssetRef<Space> &source_space, AssetRef<AssetTask> &callee, AssetRef<IpcEndpointConnection> &connection, IpcMessage *msg, bool is_call);

fc::Result<void> ipc_send_async(AssetRef<Space> &source_space, AssetRef<IpcEndpointConnection> &connection, IpcMessage *msg);

fc::Result<void> ipc_reply(AssetRef<Space> &space, AssetRef<IpcMessageReturnTask> &return_task, IpcMessage *target);

} // namespace kernel

fc::Result<AssetRef<kernel::IpcEndpoint>> query_server(IpcServerHandle handle);
