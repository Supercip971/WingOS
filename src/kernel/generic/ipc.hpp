#pragma once 

#include "kernel/generic/space.hpp"
#include "libcore/lock/lock.hpp"
#include "wingos-headers/ipc.h"



using IpcMessageServer = IpcMessage;
using IpcMessageClient = IpcMessage;

using MessageHandle = uint64_t;

struct IpcMessagePair 
{
    IpcMessageClient client; // the message from the client's point of view
    IpcMessageServer server; // the message from the server's point of view
};

struct ReceivedIpcMessage
{
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
    Asset* self; // the asset that represents this server
    core::Vec<AssetPtr> connections; // the connections to this server
};

KernelIpcServer* register_server(IpcServerHandle handle, uint64_t space_handle);
KernelIpcServer* create_server(uint64_t space_handle);


void unregister_server(IpcServerHandle handle, uint64_t space_handle);

core::Result<KernelIpcServer*> query_server(IpcServerHandle handle);


// create a connection object for the server, 

core::Result<AssetPtr> server_accept_connection(Space* space, KernelIpcServer* server);


core::Result<MessageHandle> server_send_message(IpcConnection* connection, IpcMessage message);
core::Result<MessageHandle> server_send_call(IpcConnection* connection, IpcMessage message);

core::Result<void> server_reply_message(IpcConnection* connection, MessageHandle from, IpcMessage message);

core::Result<ReceivedIpcMessage> server_receive_message(KernelIpcServer* server, IpcConnection* connection);

core::Result<ReceivedIpcMessage> client_receive_message(IpcConnection* connection);

core::Result<ReceivedIpcMessage> client_receive_response(IpcConnection* connection, MessageHandle handle);


core::Result<IpcMessage> call_server_and_wait(IpcConnection* connection, IpcMessage message);

