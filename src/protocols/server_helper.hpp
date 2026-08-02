#pragma once

#include "iol/wingos/ipc.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/optional.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/ipc.h"

namespace prot
{

typedef enum : uint64_t
{
    PROT_SIGNAL_DISCONNECT = (uint64_t)-1
} GenericProtMessage;

class ManagedServerConnectionHandler
{

protected:
    uint64_t _port;
    Wingos::RawIpcEndpoint *_endpoint; // Set by ManagedServer::do_receive() for access to the server endpoint

public:
    void set_port(uint64_t port) { this->_port = port; }

    void set_endpoint(Wingos::RawIpcEndpoint *endpoint) { this->_endpoint = endpoint; }

    uint64_t get_port() const { return _port; }

    virtual bool init() = 0;

    virtual void signal_disconnect(IpcMessage &msg) { (void)msg; };

    // return true on reply
    virtual bool call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj) = 0;

    fc::Result<void> reply(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj)
    {
        if (reply_obj.has_value())
        {

            return reply_obj->reply(&msg);
        }

        return "tried to reply to a client not expecting a reply";
    }

    fc::Result<void> reply(IpcMessage &msg, Wingos::IpcReplyObject &reply_obj)
    {
        return reply_obj.reply(&msg);
    }

    virtual ~ManagedServerConnectionHandler() = default;
};

class ManagedServer : public fc::NoCopy
{

protected:
    Wingos::RawIpcEndpoint endpoint;

    fc::UMap<uint64_t, ManagedServerConnectionHandler *> connections;

    // supporting generic
    // DISCONNECT msg
    bool support_generic_prot = true;

public:
    auto addr() const { return endpoint.published_addr; }

    virtual fc::Result<ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator)
    {
        (void)initiator;
        return "unregistered connection error";
    }

    ~ManagedServer()
    {
        connections.clear();
        endpoint.remove();
    }

    template <typename ServerImpl>
    static fc::Result<ServerImpl> create_registered_server(fc::Str name, uint64_t major = 1, uint64_t minor = 0)
    {
        ServerImpl server = {};

        auto init_conn = InitConnection::connect();
        if (init_conn.is_error())
        {
            return "failed to connect to init";
        }
        auto v = init_conn.unwrap();

        auto ipc_server = Wingos::Space::self().create_ipc_server();
        InitRegisterServer reg = {};
        name.copy_to((char *)reg.name, 80);
        reg.major = major;
        reg.minor = minor;
        reg.endpoint = ipc_server.published_addr;

        auto res = v.register_server(reg);

        if (res.is_error())
        {
            return "failed to register server with init";
        }

        server.endpoint = ipc_server;

        v.raw_client().disconnect();
        return (server);
    }

    template <typename ServerImpl = ManagedServer>
    static fc::Result<ServerImpl> create_server(bool is_root = false)
    {
        ServerImpl server = {};
        auto ipc_server = Wingos::Space::self().create_ipc_server(is_root);
        server.endpoint = ipc_server;

        return (server);
    }

    const auto &raw_server() const { return endpoint; }

    void disconnect(uint64_t port)
    {
        connections.remove(port);

        // Then disconnect from the raw server (which will delete the connection)
    }

    void close()
    {
        endpoint.remove();
    }

    fc::Result<void> do_receive()
    {
        IpcMessage msg;
        auto res = try$(endpoint.receive(&msg));

        if (!connections.has(msg.port))
        {
            auto connection = try$(on_connect(msg));
            connection->set_port(msg.port);
            connection->set_endpoint(&endpoint);
            connection->init();
            connections.insert(msg.port, connection);
        }

        if (msg.arguments.data[0].data == PROT_SIGNAL_DISCONNECT)
        {
            connections[msg.port]->signal_disconnect(msg);
            connections.remove(msg.port);
        }
        else
        {

            if (res.handle == 0)
            {
                connections[msg.port]->call_received(msg, fc::novalue);
            }
            else
            {
                connections[msg.port]->call_received(msg, res);
            }
        }

        return {};
    };

    void loop()
    {
        while (true)
        {
            do_receive();
        }
    }
};
} // namespace prot
