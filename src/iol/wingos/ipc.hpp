#pragma once

#include "asset.hpp"
#include "wingos-headers/ipc.h"
namespace Wingos

{

struct IpcConnection : public UAsset
{
};
struct MessageServerReceived
{
    IpcMessage received;
    IpcConnection *connection; // the connection that received the message
};

struct IpcServer : public UAsset
{

    uint64_t space_handle; // the space the server belongs to
    core::Vec<IpcConnection *> connections  = {};
    IpcServerHandle addr; // the adress of the server


    void disconnect(IpcConnection* connection)
    {
        for(size_t i = 0; i < connections.len(); i++)
        {
            if(connections[i]->handle == connection->handle)
            {
                connections.pop(i);
                sys$asset_release(space_handle, connection->handle);
                delete connection;
                return;
            }
        }
    }
    static IpcServer create(uint64_t space_handle, bool is_root = false)
    {
        IpcServer server = {};
        auto res = sys$ipc_create_server(space_handle, is_root);
        server.handle = res.returned_handle;
        server.addr = res.returned_addr;
        server.space_handle = space_handle;
        server.connections = {};
        return server;
    }

    core::Result<IpcConnection *> accept(bool block = false)
    {
        auto res = sys$ipc_accept(block, space_handle, this->handle);
        if (res.accepted_connection)
        {
            IpcConnection *connection = new IpcConnection();
            connection->handle = res.connection_handle;
            connections.push(connection);
            return core::Result<IpcConnection *>::success(connection);
        }
        return core::Result<IpcConnection *>::error("failed to accept connection");
    }

    core::Result<MessageServerReceived> receive(IpcConnectionHandle connection_handle, bool block = false)

    {
        IpcMessage res_message = {};
        auto res = sys$ipc_receive_server(block, space_handle, this->handle, connection_handle, &res_message);
        if (res.contain_response)
        {

            MessageServerReceived msg;
            msg.received = core::move(res_message);
            msg.received.message_id = res.returned_msg_handle;
            return core::Result<MessageServerReceived>::success(core::move(msg));
        }

        if(res.is_disconnect)
        {
            MessageServerReceived msg = {};
            msg.received.flags |= IPC_MESSAGE_FLAG_DISCONNECT;
            
            return core::Result<MessageServerReceived>::success(core::move(msg));
        }
        return core::Result<MessageServerReceived>::error("failed to receive message");
    }

    void remove()
    {
        for(size_t i = 0; i < connections.len(); i++)
        {
            sys$asset_release(space_handle, connections[i]->handle);
            delete connections[i];
        }
        sys$asset_release(space_handle, this->handle);
    }

    core::Result<MessageServerReceived> receive(bool block = false)
    {
        for (auto &connection : connections)
        {
            auto res = receive(connection->handle, block);
            if (res.is_error())
            {
                continue; // try next connection
            }
            auto v = core::move(res.unwrap());
            v.connection = connection;
            return v;
        }
        return core::Result<MessageServerReceived>::error("no connection available to receive message");
    }

    core::Result<void> reply(MessageServerReceived &&to, IpcMessage &message)
    {
        sys$ipc_reply(space_handle, this->addr, to.connection->handle, to.received.message_id, &message);
        return {};
    }
};

struct IpcClient : public UAsset
{

    public:
    uint64_t associated_space_handle; // the space the client belongs to



    static IpcClient connect(uint64_t space_handle, uint64_t server_address, bool block = false, uint64_t flags = 0)
    {
        IpcClient client;
        auto res = sys$ipc_connect(block, space_handle, server_address, flags);
        if (res.returned_handle == 0)
        {
            log::err$("failed to connect to server: {}", res.returned_handle);
            return client;
        }
        client.handle = res.returned_handle;
        client.associated_space_handle = space_handle;
        return client;
    }

    void disconnect()
    {
        sys$asset_release(associated_space_handle, handle);
    }

    bool status()
    {
        SyscallIpcStatus status = sys$ipc_status(associated_space_handle, handle);
        if (status.returned_is_accepted)
        {
            return true;
        }
        return false;
    }

    bool wait_for_accept()
    {
        while (true)
        {   
            auto res = sys$ipc_status(associated_space_handle, handle);
            if (res.returned_is_accepted)
            {
                return true;
            }
        }
        return false; // should never reach here
    }

    core::Result<MessageHandle> send(IpcMessage &message, bool expect_reply = false)
    {

        SyscallIpcSend send = sys$ipc_send(associated_space_handle, handle, &message, expect_reply);

        return core::Result<size_t>::success(send.returned_msg_handle);
    }

    core::Result<IpcMessage> call(IpcMessage &message)
    {
        IpcMessage res = {};
        SyscallIpcCall call = sys$ipc_call(associated_space_handle, handle, &message, &res);
        if (call.has_reply)
        {
            return res;
        }
        return core::Result<IpcMessage>::error("failed to call server");
    }

    core::Result<IpcMessage> receive_reply(MessageHandle message_handle, bool block = false)
    {
        IpcMessage res;
        SyscallIpcClientReceiveReply receive = sys$ipc_receive_reply_client(block, associated_space_handle, handle, message_handle, &res);
        
        if (receive.contain_response)
        {
            return res;
        }
        return core::Result<IpcMessage>::error("failed to receive reply");
    }
};

} // namespace Wingos