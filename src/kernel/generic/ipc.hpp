#pragma once

#include "kernel/generic/asset.hpp"
#include "kernel/generic/space.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/lock/lock.hpp"
#include "wingos-headers/ipc.h"


struct IpcMessageContent 
{
     uint64_t message_id; // unique id of the message
    uint64_t flags;      // flags for the message
    IpcData data[8];     // data for the message, can be used for IPC payload
   
};
using IpcMessageServer = IpcMessageContent;
using IpcMessageClient = IpcMessageContent;



struct IpcMessagePair
{
    uint64_t data[MAX_IPC_DATA_SIZE / sizeof(uint64_t)];
    uint64_t len;

    IpcMessageClient client; // the message from the client's point of view
    IpcMessageServer server; // the message from the server's point of view

    static IpcMessagePair const& from_client(IpcMessage& msg)
    {
        static IpcMessagePair pair;
        pair.client.message_id = msg.message_id;
        pair.client.flags = msg.flags;
        
        pair.server = {};
        pair.len = msg.len;
        for (size_t i = 0; i < 8; i++)
        {
            pair.data[i] = msg.data[i].data;
        }

        for (size_t i = 0; i < msg.len / sizeof(uint64_t); i++)
        {
            pair.data[i] = msg.data[i].data;
        }
        return pair;
    } 

    static IpcMessagePair const& from_server(IpcMessage& msg)
    {
        static IpcMessagePair pair;
        pair.server.message_id = msg.message_id;
        pair.server.flags = msg.flags;

        pair.client = {};
        pair.len = msg.len;
        for (size_t i = 0; i < 8; i++)
        {
            pair.data[i] = msg.data[i].data;
        }

        for (size_t i = 0; i < msg.len / sizeof(uint64_t); i++)
        {
            pair.data[i] = msg.data[i].data;
        }
        return pair;
    }

    IpcMessage to_client() const
    {
        IpcMessage msg = {};
        msg.message_id = client.message_id;
        msg.flags = client.flags;
        msg.len = len;

        for (size_t i = 0; i < 8; i++)
        {
            msg.data[i].data = data[i];
            msg.data[i].is_asset = false; // assuming no assets in this message
        }

        for (size_t i = 0; i < len / sizeof(uint64_t); i++)
        {
            msg.buffer[i] = data[i];
        }

        return msg;
    }

    IpcMessage to_server() const
    {
        IpcMessage msg = {};
        msg.message_id = server.message_id;
        msg.flags = server.flags;
        msg.len = len;

        for (size_t i = 0; i < 8; i++)
        {
            msg.data[i].data = data[i];
            msg.data[i].is_asset = false; // assuming no assets in this message
        }

        for (size_t i = 0; i < len / sizeof(uint64_t); i++)
        {
            msg.buffer[i] = data[i];
        }

        return msg;
    }
};

struct ReceivedIpcMessage
{
    bool is_null;
    bool is_call;
    bool has_reply; // if true, the message has a reply
    bool has_been_received;
    uint64_t server_id;
    MessageHandle uid;
    IpcMessagePair message_sended;
    IpcMessagePair message_responded; // the message from the pov of the server (handle are different)
};

struct IpcConnection
{
    uint64_t message_alloc_id;
    core::Lock lock;
    uint64_t client_space_handle; // the space handle of the client that created this connection
    uint64_t server_space_handle; // the space handle of the server that created this connection

    bool accepted;
    IpcServerHandle server_handle; // the handle of the server that this connection is connected to

    core::Vec<ReceivedIpcMessage> message_sent;
};

struct KernelIpcServer
{

    core::Lock lock;
    IpcServerHandle handle;
    uint64_t parent_space;
    Asset *self;                     // the asset that represents this server
    core::Vec<AssetPtr> connections; // the connections to this server
};

KernelIpcServer *register_server(IpcServerHandle handle, uint64_t space_handle);
KernelIpcServer *create_server(uint64_t space_handle);

void unregister_server(IpcServerHandle handle, uint64_t space_handle);

core::Result<KernelIpcServer *> query_server(IpcServerHandle handle);

// create a connection object for the server,

core::Result<AssetPtr> server_accept_connection(KernelIpcServer *server);

core::Result<MessageHandle> server_send_message(IpcConnection *connection, IpcMessage* message, bool expect_reply = false);
core::Result<MessageHandle> server_send_call(IpcConnection *connection, IpcMessage* message);

core::Result<void> server_reply_message(IpcConnection *connection, MessageHandle from, IpcMessage* message);

core::Result<ReceivedIpcMessage> server_receive_message(KernelIpcServer *server, IpcConnection *connection);

core::Result<ReceivedIpcMessage> client_receive_message(IpcConnection *connection);

core::Result<ReceivedIpcMessage> client_receive_response(IpcConnection *connection, MessageHandle handle);

core::Result<IpcMessage> call_server_and_wait(IpcConnection *connection, IpcMessage* message);
