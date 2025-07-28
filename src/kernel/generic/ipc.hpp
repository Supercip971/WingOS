#pragma once 

#include "kernel/generic/space.hpp"
#include "wingos-headers/ipc.h"

struct ReceivedIpcMessage
{
    bool is_call; 
    bool has_reply; // if true, the message has a reply
    uint64_t message_id; // the id of the message
    uint64_t from_id; 
    IpcMessage message_sended;
    IpcMessage message_responded; // the message from the pov of the server (handle are different)
    
};

struct IpcConnection 
{
    uint64_t client_space_handle; // the space handle of the client that created this connection
    uint64_t server_space_handle; // the space handle of the server that created this connection

    IpcServerHandle server_handle; // the handle of the server that this connection is connected to

    core::Vec<ReceivedIpcMessage> message_sent;
};

struct KernelIpcServer 
{
    IpcServerHandle handle;
    uint64_t parent_space; 
    
    core::Vec<Asset> connections; // the connections to this server
};

