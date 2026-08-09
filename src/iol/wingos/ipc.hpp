#pragma once

#include "asset.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "wingos-headers/ipc.h"
#include "wingos-headers/syscalls.h"

namespace Wingos

{

struct IpcReplyObject : public UAsset
{
    uint64_t space_handle;

    fc::Result<void> reply(IpcMessage *message)
    {
        sys$ipc_reply(space_handle, handle, message);
        return {};
    }
};

struct RawIpcEndpoint : public UAsset
{

    uint64_t space_handle;          // the space the server belongs to
    IpcServerHandle published_addr; // the adress of the server

    static RawIpcEndpoint create(uint64_t space_handle, bool publish = false, bool is_root = false)
    {
        RawIpcEndpoint server = {};
        auto res = sys$ipc_create_endpoint(space_handle, is_root, publish);
        server.handle = res.returned_handle;
        server.published_addr = res.returned_addr;
        server.space_handle = space_handle;
        return server;
    }

    fc::Result<IpcReplyObject> receive(IpcMessage *ret_msg, bool block = true)
    {
        IpcReplyObject rcv_msg{};
        auto r = sys$ipc_receive(
            space_handle, this->handle,
            ret_msg,
            !block);

        rcv_msg.handle = r.return_context_handle;
        rcv_msg.space_handle = space_handle;

        return rcv_msg;
    }

    void remove()
    {
        sys$asset_release(space_handle, this->handle);
    }
};

struct IpcClient : public UAsset
{

public:
    uint64_t port;
    uint64_t associated_space_handle; // the space the client belongs to

    static IpcClient from(uint64_t space_handle, uint64_t connection, uint64_t port)
    {
        IpcClient client = {};
        client.handle = connection;
        client.port = port;
        client.associated_space_handle = space_handle;
        return client;
    }

    static IpcClient from(uint64_t space_handle, uint64_t endpoint_handle)
    {
        return IpcClient::already_connected(space_handle, endpoint_handle);
    }

    static IpcClient connect_by_addr(uint64_t space_handle, uint64_t endpoint_address)
    {
        IpcClient client = {};
        auto res = sys$ipc_connect(space_handle, true, endpoint_address);
        if (res.returned_handle_sender == 0)
        {
            fmt::err$("failed to connect to server: {}", res.returned_handle_sender);
            return client;
        }
        client.handle = res.returned_handle_sender;
        client.port = res.port_used;
        client.associated_space_handle = space_handle;
        return client;
    }

    static IpcClient already_connected(uint64_t space_handle, uint64_t endpoint_handle)
    {
        IpcClient client = {};
        client.handle = endpoint_handle;
        client.associated_space_handle = space_handle;
        auto info = sys$ipc_asset_info(space_handle, endpoint_handle);
        client.port = info.returned_info.connection.port;

        return client;
    }

    static IpcClient connect_to_object(uint64_t space_handle, uint64_t endpoint_handle)
    {
        IpcClient client = {};
        auto res = sys$ipc_connect(space_handle, false, endpoint_handle);
        if (res.returned_handle_sender == 0)
        {
            fmt::err$("failed to connect to server: {}", res.returned_handle_sender);
            return client;
        }
        client.handle = res.returned_handle_sender;
        client.port = res.port_used;
        client.associated_space_handle = space_handle;
        return client;
    }

    void disconnect(bool signal = true)
    {
        if (signal)
        {
            IpcMessage disc = {};
            disc.arguments.data[0].data = -1;
            sys$ipc_send(associated_space_handle, handle, &disc, true);
        }
        sys$asset_release(associated_space_handle, handle);
    }

    fc::Result<void> send_async(IpcMessage &message)
    {
        sys$ipc_send(associated_space_handle, handle, &message, true);
        return {};
    }

    fc::Result<void> send(IpcMessage &message)
    {
        sys$ipc_send(associated_space_handle, handle, &message, false);
        return {};
    }

    fc::Result<void> call(IpcMessage &message_inplace)
    {
        sys$ipc_call(associated_space_handle, handle, &message_inplace);
        return {};
    }
};

} // namespace Wingos
