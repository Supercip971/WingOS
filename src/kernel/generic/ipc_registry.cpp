#include "kernel/generic/ipc_registry.hpp"

#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/ipc_asset.hpp"
#include <arch/x86_64/barrier.hpp>

#include "asset.hpp"
#include "kernel/generic/asset.hpp"
#include "kernel/generic/blocker.hpp"
#include "kernel/generic/scheduler.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/logic.hpp"
#include "libcore/result.hpp"
#include "scheduler.hpp"
#include "space.hpp"
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

RegisteredServer *register_server(IpcServerHandle handle, AssetRef<kernel::IpcEndpoint> &server)
{
    auto registered_server = new RegisteredServer();
    registered_server->handle = handle;
    registered_server->server = server;

    ipc_server_lock.lock();
    registered_servers.push(registered_server);
    ipc_server_lock.release();
    return registered_server;
}

RegisteredServer *create_registered_server()
{
    auto server = new RegisteredServer();
    server->handle = next_free_ipc_server_handle++;

    ipc_server_lock.lock();
    registered_servers.push(server);
    ipc_server_lock.release();

    return server;
}

// Allocate a KernelIpcServer WITHOUT making it globally visible.
// Caller MUST call publish_server() after setting server->self and all
// other fields.  This two-phase init prevents SMP races where another CPU
// queries the server (via query_server / query_server_locked) before
// server->self is initialized
kernel::IpcEndpoint *allocate_server(AssetRef<Space> &attached_handle)
{
    auto *server = new kernel::IpcEndpoint();
    server->target_message_space = attached_handle;
    return server;
}

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

void release_server_lock()
{
    ipc_server_lock.release();
}

namespace kernel
{

void unregister_server(IpcServerHandle handle, uint64_t space_handle)
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
    fmt::warn$("unregister_server: server not found: {} {}", handle, space_handle);
}

uint64_t get_next_ipc_server_handle()
{
    return next_free_ipc_server_handle++;
}

} // namespace kernel
