#include "service_register.hpp"

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/fmt/log.hpp"
#include "wingos-headers/ipc.h"
#include "protocols/init/init.hpp"

core::Vec<Wingos::IpcConnection *> connections;


struct RegisteredService {

    char name[80];
    uint64_t major;
    uint64_t minor;
    uint64_t endpoint;
} ;


core::Vec<RegisteredService*> registered_services;

void startup_init_service(Wingos::IpcServer server)
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
            
            switch(msg.received.data[0].data)
            {
                case prot::INIT_REGISTER_SERVER:
                {
                    RegisteredService *service = new RegisteredService();
                    size_t i;
                    for( i = 0; i < 80-1 && i < msg.received.len; i++)
                    {
                        service->name[i] = msg.received.raw_buffer[i];
                    }
                    service->name[i] = 0;
                    
                    service->endpoint = msg.received.data[1].data;
                    service->major = msg.received.data[2].data;
                    service->minor = msg.received.data[3].data;

                    registered_services.push(service);


                    log::log$("(server) registered service: {} ({}.{}) at {}", service->name, service->major, service->minor, service->endpoint);
                    
                    service_startup_callback(service->name);
                    break;
                }
                case prot::INIT_UNREGISTER_SERVER:
                {
                    log::warn$("(server) unregister server not implemented yet");
                    break;
                }
                case prot::INIT_GET_SERVER:
                {
                    prot::InitGetServerResponse resp {};
                    resp.endpoint = 0;

                    core::Str b = core::Str((char*)msg.received.raw_buffer, msg.received.len-1);
                    for(size_t j = 0; j < registered_services.len(); j++)
                    {
                        auto &service = registered_services[j];
                        bool name_match = true;

                        core::Str a = core::Str(service->name);
                        name_match = (a == b);
                        if(name_match && service->major == msg.received.data[1].data && service->minor >= msg.received.data[2].data)
                        {
                            resp.endpoint = service->endpoint;
                            resp.major = service->major;
                            resp.minor = service->minor;
                            break;
                        }    
                    }

                    if(resp.endpoint == 0)
                    {

                        core::Str v = core::Str((char*)msg.received.raw_buffer, msg.received.len-1);
                     
                        log::log$("(server) service not found: {} ( {} . {} )", v, msg.received.data[1].data, msg.received.data[2].data);
                        
                        for(size_t j = 0; j < registered_services.len(); j++)
                        {
                            auto &service = registered_services[j];
                            log::log$("(server) available service: {} ({}.{}) at {}", service->name, service->major, service->minor, service->endpoint);
                        }
                    }
                    else
                    {
                        log::log$("(server) found get server request: {} ({}.{}) -> {}",b, msg.received.data[1].data, msg.received.data[2].data, resp.endpoint);
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
                default:
                {
                    log::log$("(server) unknown message type: {}", msg.received.data[0].data);
                    break;
                }
            }
        }
    }
}