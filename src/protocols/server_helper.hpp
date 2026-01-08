#pragma once

#include "protocols/init/init.hpp"

namespace prot
{
class ManagedServer : public core::NoCopy
{

    IpcServerHandle self_endpoint;
    Wingos::IpcServer ipc_server;

    bool loaded = false;

public:
    auto addr() const { return self_endpoint; }

    size_t connection_count() const { return ipc_server.connections.len(); }
    static core::Result<ManagedServer> create_registered_server(core::Str name, uint64_t major = 1, uint64_t minor = 0)
    {
        ManagedServer server = {};

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
        reg.endpoint = ipc_server.addr;

        auto res = v.register_server(reg);


        if (res.is_error())
        {
            return "failed to register server with init";
        }

        server.self_endpoint = ipc_server.addr;

        server.ipc_server = (ipc_server);
        server.loaded = true;
        // TODO:
        // v.disconnect();
        return (server);
    }

    static core::Result<ManagedServer> create_server()
    {
        ManagedServer server = {};
        auto ipc_server = Wingos::Space::self().create_ipc_server();
        server.self_endpoint = ipc_server.addr;
        server.ipc_server = (ipc_server);
        server.loaded = true;

        return (server);
    }

    bool accept_connection()
    {

        if(!loaded)
        {
            log::err$("ManagedServer: accept_connection called before server was fully initialized");
            while(true){};
        }

        auto c = ipc_server.accept();

        if(c.is_error())
        {
            return false;
        }
        return true;
    }

    Wingos::IpcServer &raw_server() { return ipc_server; }

    void disconnect(Wingos::IpcConnection *connection)
    {

        // Then disconnect from the raw server (which will delete the connection)
        ipc_server.disconnect(connection);
    }

    void close()
    {
        if(!loaded)
        {
            log::err$("ManagedServer: close called before server was fully initialized");
            return;
        }

        // Clear our local list but don't delete - ipc_server.remove() will handle
        // releasing assets and deleting the IpcConnection objects

        ipc_server.remove();
    }

    core::Result<Wingos::MessageServerReceived> try_receive()
    {

        auto msg = ipc_server.receive();

        if (msg.is_error())
        {
            return core::Result<Wingos::MessageServerReceived>::error("no message received");
        }

        return msg;
    }

    core::Result<void> reply(Wingos::MessageServerReceived &&to, IpcMessage &message)
    {
        return ipc_server.reply(core::move(to), message);
    }
};
} // namespace prot
