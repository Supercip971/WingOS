#pragma once

#include "kernel/generic/blocker.hpp"
#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/space.hpp"
#include "math/align.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/lock/lock.hpp"
#include "scheduler.hpp"
#include "wingos-headers/ipc.h"

// Forward declarations to avoid pulling `space.hpp` (and its dependencies) into IPC.
struct Space;

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
    uint64_t buffer[MAX_IPC_BUFFER_SIZE / sizeof(uint64_t)];
    uint64_t len;


    IpcMessageClient client; // the message from the client's point of view
    IpcMessageServer server; // the message from the server's point of view

    IpcMessagePair() : buffer{}, len(0), client{}, server{}
    {
    }

    IpcMessagePair(const IpcMessagePair &other) {
        len = other.len;
        client = other.client;
        server = other.server;
        for (size_t i = 0; i < math::alignUp((uint64_t)other.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
        {
            buffer[i] = other.buffer[i];
        }
    }

    IpcMessagePair &operator=(const IpcMessagePair &other) {
        if (this != &other) {
            len = other.len;
            client = other.client;
            server = other.server;
            for (size_t i = 0; i < math::alignUp((uint64_t)other.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
            {
                buffer[i] = other.buffer[i];
            }
        }
        return *this;
    }

    IpcMessagePair(IpcMessagePair &&other) noexcept {
        len = other.len;
        client = other.client;
        server = other.server;
        for (size_t i = 0; i < math::alignUp((uint64_t)other.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
        {
            buffer[i] = other.buffer[i];
        }
    }

    IpcMessagePair &operator=(IpcMessagePair &&other) noexcept {
        if (this != &other) {
            len = other.len;
            client = other.client;
            server = other.server;
            for (size_t i = 0; i < math::alignUp((uint64_t)other.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
            {
                buffer[i] = other.buffer[i];
            }
        }
        return *this;
    }

    static IpcMessagePair from_client(IpcMessage &msg)
    {
        IpcMessagePair pair;
        pair.client.message_id = msg.message_id;
        pair.client.flags = msg.flags;

        pair.server = {};
        pair.len = msg.len;
        for (size_t i = 0; i < 8; i++)
        {
            pair.client.data[i] = msg.data[i];
        }

        for (size_t i = 0; i < math::alignUp((uint64_t)msg.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
        {
            pair.buffer[i] = msg.buffer[i];
        }
        return pair;
    }

    static IpcMessagePair from_server(IpcMessage &msg)
    {
        IpcMessagePair pair;
        pair.server.message_id = msg.message_id;
        pair.server.flags = msg.flags;

        pair.client = {};
        pair.len = msg.len;
        for (size_t i = 0; i < 8; i++)
        {
            pair.server.data[i] = msg.data[i];
        }

        for (size_t i = 0; i < math::alignUp((unsigned long)msg.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
        {
            pair.buffer[i] = msg.buffer[i];
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
            msg.data[i] = client.data[i];
        }

        for (size_t i = 0; i < math::alignUp(len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
        {
            msg.buffer[i] = buffer[i];
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
            msg.data[i] = server.data[i];
        }

        for (size_t i = 0; i < math::alignUp(len, sizeof(uint64_t))/ sizeof(uint64_t); i++)
        {
            msg.buffer[i] = buffer[i];
        }

        return msg;
    }
};

struct ReceivedIpcMessage
{
    bool is_null;
    bool is_call;
    bool is_disconnect;
    bool has_reply; // if true, the message has a reply
    bool has_been_received;
    uint64_t server_id;
    MessageHandle uid;
    IpcMessagePair message_sended;
    IpcMessagePair message_responded; // the message from the pov of the server (handle are different)
};


typedef enum
{
    IPC_STILL_OPEN = 0,
    IPC_CLOSED = 1 << 1,
} IpcConnectionClosedStatus;

struct KernelIpcServer;
struct IpcConnection : public Asset
{
    uint64_t message_alloc_id;
    core::Lock lock;

    // FIXME: store maybe space as ptr for performance ?
    uint64_t client_space_handle; // the space handle of the client that created this connection
    uint64_t server_space_handle; // the space handle of the server that created this connection

    bool accepted;
    IpcConnectionClosedStatus closed_status;
    KernelIpcServer *server;
    AssetRef<> server_asset; // only made to keep the server alive
    IpcServerHandle server_handle;

    kernel::BlockMutex client_mutex;
    kernel::BlockMutex server_mutex;
    core::Vec<ReceivedIpcMessage> message_sent;

    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_IPC_CONNECTION;

    IpcConnection() :
        Asset(AssetKind::OBJECT_KIND_IPC_CONNECTION),
        message_alloc_id(0),
        lock(),
        client_space_handle(0),
        server_space_handle(0),
        accepted(false),
        closed_status(IPC_STILL_OPEN),
        server(nullptr),
        server_asset(),
        server_handle(0),
        client_mutex(),
        server_mutex(),
        message_sent()
    {   }
};

struct KernelIpcServer
{

    IpcServerHandle handle;
    uint64_t parent_space;
    AssetRef<> self;              // the asset that represents this server
    core::Vec<AssetRef<>> connections; // the connections to this server (untyped; returned by Space::asset_copy)
    bool destroyed;
};

KernelIpcServer *register_server(IpcServerHandle handle, uint64_t space_handle);
KernelIpcServer *create_server(uint64_t space_handle);

// Allocate a KernelIpcServer struct WITHOUT registering it globally.
// The caller MUST call publish_server() after fully initializing the server
KernelIpcServer *allocate_server(IpcServerHandle handle, uint64_t space_handle);
KernelIpcServer *allocate_server_auto_handle(uint64_t space_handle);

// Register a fully-initialized KernelIpcServer in the global server list.
void publish_server(KernelIpcServer *server);

void unregister_server(IpcServerHandle handle, uint64_t space_handle);

core::Result<KernelIpcServer *> query_server(IpcServerHandle handle);

// This prevents the server from being unregistered/deleted while the caller uses it.
core::Result<KernelIpcServer *> query_server_locked(IpcServerHandle handle);
void release_server_lock();

// create a connection object for the server,
// Note: keep this API untyped to avoid requiring `AssetConnection` in this header.
core::Result<AssetRef<>> server_accept_connection(KernelIpcServer *server);

core::Result<MessageHandle> server_send_message(AssetRef<AssetConnection>& connection, IpcMessage *message, bool expect_reply = false);
core::Result<MessageHandle> server_send_call(AssetRef<AssetConnection> &connection, IpcMessage *message);

core::Result<void> server_reply_message(AssetRef<AssetConnection>& connection, MessageHandle from, IpcMessage *message);

core::Result<ReceivedIpcMessage> server_receive_message( AssetRef<AssetConnection>& connection);

core::Result<ReceivedIpcMessage> client_receive_message(AssetRef<AssetConnection>& connection);

core::Result<ReceivedIpcMessage> client_receive_response(AssetRef<AssetConnection>& connection, MessageHandle handle);

core::Result<IpcMessage> call_server_and_wait(AssetRef<AssetConnection>& connection, IpcMessage *message);
