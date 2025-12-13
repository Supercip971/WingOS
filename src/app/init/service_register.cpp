#include "service_register.hpp"

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/fmt/log.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/ipc.h"

core::Vec<Wingos::IpcConnection *> connections = {};

struct RegisteredService
{

    char name[80];
    uint64_t major;
    uint64_t minor;
    uint64_t endpoint;
};

core::Vec<RegisteredService *> registered_services = {};

core::Result<void> service_register(uint64_t endpoint, core::Str const &name, uint64_t major, uint64_t minor)

{
    log::log$("registering service: {} ({}.{}) at {}", name, major, minor, endpoint);
    RegisteredService *service = new RegisteredService();
    size_t i;
    for (i = 0; i < 80 - 1 && i < name.len(); i++)
    {
        service->name[i] = name[i];
    }
    service->name[i] = 0;

    service->endpoint = endpoint;
    service->major = major;
    service->minor = minor;

    registered_services.push(service);

    log::log$("(server) registered service: {} ({}.{}) at {}", service->name, service->major, service->minor, service->endpoint);

    service_startup_callback(service->name);

    return {};
}

core::Result<IpcServerHandle> service_get(core::Str const &name, uint64_t major, uint64_t minor)
{
    for (size_t j = 0; j < registered_services.len(); j++)
    {
        auto &service = registered_services[j];
        bool name_match = true;

        name_match = (core::Str(service->name) == name);
        if (name_match && service->major == major && service->minor >= minor)
        {
            return {service->endpoint};
        }
    }

    log::log$("service not found: {} ({}.{})", name, major, minor);

    return ("service not found");
}

void startup_init_service(Wingos::IpcServer server, MachineContextShared shared)
{

    log::log$("created init server with handle: {}", server.handle);

    registered_services = {};

    while (true)
    {

        auto conn = server.accept();
        if (!conn.is_error())
        {
            log::log$("(server) accepted connection: {}", conn.unwrap()->handle);
            connections.push(conn.unwrap());
        }

        
        auto received = server.receive();

        if (!received.is_error())
        {
            auto msg = received.take();

            if(msg.received.flags & IPC_MESSAGE_FLAG_DISCONNECT)
            {
                log::log$("(server) disconnecting connection");
                server.disconnect(msg.connection);
                continue;
            }
            switch (msg.received.data[0].data)
            {
            case prot::INIT_REGISTER_SERVER:
            {
                service_register(
                    msg.received.data[1].data,
                    core::Str((char *)msg.received.raw_buffer, msg.received.len - 1),
                    msg.received.data[2].data,
                    msg.received.data[3].data)
                    .assert();
                break;
            }
            case prot::INIT_UNREGISTER_SERVER:
            {
                log::warn$("(server) unregister server not implemented yet");
                break;
            }
            case prot::INIT_GET_SERVER:
            {
                prot::InitGetServerResponse resp{};
                resp.endpoint = 0;

                auto name_len = msg.received.len - 1;
                core::Str name = core::Str((char *)msg.received.raw_buffer, name_len);

                auto service_res = service_get(
                    name,
                    msg.received.data[1].data,
                    msg.received.data[2].data);

                if (!service_res.is_error())
                {
                    resp.endpoint = service_res.unwrap();
                }
                else
                {
                    log::log$("(server) get server failed: {}", service_res.error());
                }

                IpcMessage reply = {};
                reply.data[0].data = resp.endpoint;
                reply.data[0].is_asset = false;
                server.reply(core::move(msg), reply).assert();
                break;
            }
            case prot::INIT_SIGNAL_FS_AVAILABLE:
            {
                log::log$("(server) received signal fs available");
                service_startup_callback("@fs");
                break;
            }

            case prot::INIT_QUERY_FB:
            {
                prot::InitQueryFbResponse resp{};
                resp.framebuffer_addr = shared.framebuffer_addr;
                resp.framebuffer_width = shared.framebuffer_width;
                resp.framebuffer_height = shared.framebuffer_height;

                IpcMessage reply = {};
                reply.data[0].data = resp.framebuffer_addr;
                reply.data[1].data = resp.framebuffer_width;
                reply.data[2].data = resp.framebuffer_height;
                server.reply(core::move(msg), reply).assert();
                break;
            }
            default:
            {
                log::log$("(server) unknown message type: {}", msg.received.data[0].data);
                break;
            }
            }
        }
    }
}