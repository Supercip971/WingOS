#include "kernel/generic/ipc_registry.hpp"

#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/ipc_asset.hpp"

#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

struct RegisteredServer
{
    IpcServerHandle handle;
    AssetRef<kernel::IpcEndpoint> server;

    RegisteredServer() = default;
};

static uint64_t next_free_ipc_server_handle = 16;

static fc::Lock ipc_server_lock;
static fc::Vec<RegisteredServer *> registered_servers = {};

namespace kernel
{

IpcServerHandle publish_server(AssetRef<kernel::IpcEndpoint> &endpoint, bool is_root)
{
    ipc_server_lock.lock();

    auto server = new RegisteredServer();
    server->handle = (is_root ? 0 : next_free_ipc_server_handle++);
    server->server = endpoint;

    registered_servers.push(server);

    ipc_server_lock.release();
    return server->handle;
}

} // namespace kernel

fc::Result<AssetRef<kernel::IpcEndpoint>> query_server(IpcServerHandle handle)
{
    ipc_server_lock.lock();
    for (size_t i = 0; i < registered_servers.len(); i++)
    {
        if (registered_servers[i]->handle == handle)
        {
            auto server = registered_servers[i]->server;
            ipc_server_lock.release();
            return server;
        }
    }
    ipc_server_lock.release();

    fmt::warn$("query_server: server not found: {}", handle);
    return ("server not found");
}

namespace kernel
{

void unregister_server(IpcServerHandle handle, AssetHandle space_handle)
{
    ipc_server_lock.lock();
    for (size_t i = 0; i < registered_servers.len(); i++)
    {
        if (registered_servers[i]->handle == handle && registered_servers[i]->server->target_message_space.handle == space_handle)
        {
            registered_servers.pop(i);
            ipc_server_lock.release();
            return;
        }
    }
    ipc_server_lock.release();
    fmt::warn$("unregister_server: server not found: {} {}", handle, space_handle.id());
}

uint64_t get_next_ipc_server_handle()
{
    return next_free_ipc_server_handle++;
}

} // namespace kernel
