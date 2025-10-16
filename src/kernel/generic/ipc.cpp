#include "ipc.hpp"

#include "kernel/generic/asset.hpp"
#include "kernel/generic/space.hpp"
#include "libcore/ds/vec.hpp"
#include "wingos-headers/ipc.h"

struct KernelIpcServerRegistered
{
    IpcServerHandle handle;
    KernelIpcServer *server;
};

uint64_t next_free_ipc_server_handle = 16;

core::Lock ipc_server_lock;
core::Vec<KernelIpcServerRegistered> registered_servers = {};

KernelIpcServer *register_server(IpcServerHandle handle, uint64_t space_handle)
{
    KernelIpcServer *server = new KernelIpcServer();
    *server = {};
    server->handle = handle;

    server->parent_space = space_handle;
    server->lock.lock();
    server->connections.clear();
    server->self = nullptr; // will be set later when the asset is created
    server->lock.release();

    ipc_server_lock.lock();
    registered_servers.push({server->handle, server});
    ipc_server_lock.release();
    return server;
}

KernelIpcServer *create_server(uint64_t space_handle)
{
    KernelIpcServer *server = new KernelIpcServer();
    *server = {};
    server->handle = next_free_ipc_server_handle++;
    server->parent_space = space_handle;
    server->lock.lock();
    server->connections.clear();
    server->self = nullptr; // will be set later when the asset is created
    server->lock.release();

    ipc_server_lock.lock();
    registered_servers.push({server->handle, server});
    ipc_server_lock.release();

    return server;
}

core::Result<KernelIpcServer *> query_server(IpcServerHandle handle)
{
    ipc_server_lock.lock();
    for (size_t i = 0; i < registered_servers.len(); i++)
    {
        if (registered_servers[i].handle == handle)
        {
            ipc_server_lock.release();
            return registered_servers[i].server;
        }
    }
    ipc_server_lock.release();
    return core::Result<KernelIpcServer *>::error("server not found");
}

void unregister_server(IpcServerHandle handle, uint64_t space_handle)
{
    ipc_server_lock.lock();
    for (size_t i = 0; i < registered_servers.len(); i++)
    {
        if (registered_servers[i].handle == handle && registered_servers[i].server->parent_space == space_handle)
        {
            delete registered_servers[i].server;
            registered_servers.pop(i);
            ipc_server_lock.release();
            return;
        }
    }
    ipc_server_lock.release();
    log::warn$("unregister_server: server not found: {} {}", handle, space_handle);
}

core::Result<AssetPtr> server_accept_connection(KernelIpcServer *server)
{
    if (server == nullptr)
    {
        return core::Result<AssetPtr>::error("server is null");
    }

    server->lock.lock();
    for (size_t i = 0; i < server->connections.len(); i++)
    {
        if (server->connections[i].asset->ipc_connection->accepted == false)
        {
            server->connections[i].asset->ipc_connection->accepted = true;
            server->connections[i].asset->ref_count++;
            server->lock.release();
            return server->connections[i];
        }
    }
    server->lock.release();

    return core::Result<AssetPtr>::error("no connection available");
}

// send message to the server
core::Result<MessageHandle> _server_send_message(IpcConnection *connection, IpcMessage *message, bool is_call)
{
    if (connection == nullptr)
    {
        return core::Result<MessageHandle>("connection is null");
    }

    if (connection->accepted == false)
    {
        return core::Result<MessageHandle>("connection is not accepted");
    }

    ReceivedIpcMessage received_message = {};
    received_message.is_call = is_call; 
    received_message.has_reply = false;
   
    received_message.server_id = connection->server_handle; // the space handle of the client that created this connection
    received_message.message_sended = IpcMessagePair::from_client(*message);

    connection->lock.lock();
    received_message.uid = connection->message_alloc_id++; // TODO: set this to a unique id

    connection->message_sent.push(received_message);
    connection->lock.release();

    return received_message.uid; // return the unique id of the message
}

core::Result<MessageHandle> server_send_message(IpcConnection *connection, IpcMessage *message, bool expect_reply)
{

    //
    return _server_send_message(connection, message, expect_reply);
}

// for now share the same code, but for later, we will have to differentiate between call and message
// a call expects a reply and thus we can use some scheduling tricks to directly
// handle the reply by jumping onto the server code
core::Result<MessageHandle> server_send_call(IpcConnection *connection, IpcMessage *message)
{
    return _server_send_message(connection, message, true);
}

core::Result<IpcMessageServer> update_handle_from_client_to_server(IpcConnection *connection, IpcMessageClient message)
{
    if (connection == nullptr)
    {
        return core::Result<IpcMessageServer>::error("server or connection is null");
    }

    auto client_space_res = Space::global_space_by_handle(connection->client_space_handle);
    auto server_space_res = Space::global_space_by_handle(connection->server_space_handle);
    if (client_space_res.is_error() || server_space_res.is_error())
    {
        return core::Result<IpcMessageServer>::error("unable to get client or server space");
    }

    auto client_space = client_space_res.unwrap();
    auto server_space = server_space_res.unwrap();

    for (size_t i = 0; i < 8; i++)
    {
        if (message.data[i].is_asset)
        {
            auto asset_handle = message.data[i].asset_handle;

            auto asset_ptr_res = Asset::by_handle_ptr(client_space, asset_handle);
            if (asset_ptr_res.is_error())
            {
                return core::Result<IpcMessageServer>::error("asset not found in server space");
            }

            auto asset_ptr = asset_ptr_res.unwrap();

            message.data[i].asset_handle = try$(asset_move(client_space, server_space, asset_ptr)).handle;
        }
    }

    return core::Result<IpcMessageServer>::csuccess(message);
}
core::Result<IpcMessageClient> update_handle_from_server_to_client(IpcConnection *connection, IpcMessageServer message)
{
    if (connection == nullptr)
    {
        return core::Result<IpcMessageClient>::error("server or connection is null");
    }
    auto client_space_res = Space::global_space_by_handle(connection->client_space_handle);
    auto server_space_res = Space::global_space_by_handle(connection->server_space_handle);
    if (client_space_res.is_error() || server_space_res.is_error())
    {
        return core::Result<IpcMessageClient>::error("unable to get client or server space");
    }
    auto client_space = client_space_res.unwrap();
    auto server_space = server_space_res.unwrap();

    for (size_t i = 0; i < 8; i++)
    {
        if (message.data[i].is_asset)
        {
            auto asset_handle = message.data[i].asset_handle;

            auto asset_ptr_res = Asset::by_handle_ptr(server_space, asset_handle);

            if (asset_ptr_res.is_error())
            {
                return core::Result<IpcMessageClient>::error("asset not found in server space");
            }

            auto asset_ptr = asset_ptr_res.unwrap();

            message.data[i].asset_handle = try$(asset_move(server_space, client_space, asset_ptr)).handle;
        }
    }

    return core::Result<IpcMessageClient>::csuccess(message);
}

core::Result<ReceivedIpcMessage> server_receive_message(KernelIpcServer *server, IpcConnection *connection)
{
    if (server == nullptr)
    {
        return core::Result<ReceivedIpcMessage>::error("server is null");
    }

    if (connection->server_handle != server->handle)
    {
        return core::Result<ReceivedIpcMessage>::error("connection is not connected to this server");
    }

    server->lock.lock();
    for (size_t i = 0; i < connection->message_sent.len(); i++)
    {
        if (!connection->message_sent[i].has_been_received)
        {
            connection->message_sent[i].has_been_received = true;

            auto message = connection->message_sent[i];
            if (!connection->message_sent[i].is_call)
            {
                connection->message_sent.pop(i);
            }
            server->lock.release();
            message.is_null = false;

            message.message_sended.server = try$(update_handle_from_client_to_server(connection, message.message_sended.client));
            return message;
        }
    }

    server->lock.release();

    ReceivedIpcMessage null_message = {};
    null_message.is_null = true;
    return null_message;
}

core::Result<ReceivedIpcMessage> client_receive_message(IpcConnection *connection)
{
    if (connection == nullptr)
    {
        return core::Result<ReceivedIpcMessage>::error("connection is null");
    }

    connection->lock.lock();
    for (size_t i = 0; i < connection->message_sent.len(); i++)
    {
        if (connection->message_sent[i].has_reply)
        {
            auto message = connection->message_sent[i];
            if (!connection->message_sent[i].is_call)
            {
                connection->message_sent.pop(i);
            }
            connection->lock.release();

            message.message_sended.client = try$(update_handle_from_server_to_client(connection, message.message_sended.server));
            return message;
        }
    }

    connection->lock.release();

    return core::Result<ReceivedIpcMessage>::error("no message found");
}
core::Result<ReceivedIpcMessage> client_receive_response(IpcConnection *connection, MessageHandle handle)
{
    if (connection == nullptr)
    {
        return core::Result<ReceivedIpcMessage>::error("connection is null");
    }

    connection->lock.lock();
    for (size_t i = 0; i < connection->message_sent.len(); i++)
    {
        if (connection->message_sent[i].uid == handle && connection->message_sent[i].has_reply)
        {
            auto message = connection->message_sent[i];
            connection->message_sent.pop(i);
            connection->lock.release();

            message.message_responded.client = try$(update_handle_from_server_to_client(connection, message.message_responded.server));
            return message;
        }
    }

    connection->lock.release();

    ReceivedIpcMessage null_message = {};
    null_message.is_null = true;
    return null_message;
}

core::Result<void> server_reply_message(IpcConnection *connection, MessageHandle from, IpcMessage *message)
{

    if (connection == nullptr)
    {
        return core::Result<void>("connection is null");
    }

    if (!connection->accepted)
    {
        return core::Result<void>("connection is not accepted");
    }

    connection->lock.lock();
    for (size_t i = 0; i < connection->message_sent.len(); i++)
    {
        auto &from_ref = connection->message_sent[i];
        if (from_ref.uid == from)
        {
            from_ref.has_reply = true;
            from_ref.message_responded = IpcMessagePair::from_server(*message);
            from_ref.has_been_received = true;
            connection->lock.release();
            return {};
        }
    }
    connection->lock.release();

    return core::Result<void>("message not found in connection");
}

core::Result<IpcMessage> call_server_and_wait(IpcConnection *connection, IpcMessage *message)
{
    if (connection == nullptr)
    {
        return core::Result<IpcMessage>::error("connection is null");
    }

    if (!connection->accepted)
    {
        return core::Result<IpcMessage>::error("connection is not accepted");
    }

    auto res = try$(server_send_call(connection, message));

    auto msg = try$(client_receive_response(connection, res));

    return msg.message_responded.to_client();
}
