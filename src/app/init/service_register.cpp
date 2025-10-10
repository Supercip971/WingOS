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


core::Vec<RegisteredService> registered_services;

void startup_init_service(Wingos::IpcServer server)
{

    log::log$("created init server with handle: {}", server.handle);

    
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
            auto msg = received.unwrap();
            
            switch(msg.received.data[0].data)
            {
                case prot::INIT_REGISTER_SERVER:
                {
                    RegisteredService service {};
                    for(size_t i = 0; i < 80 && msg.received.raw_buffer[i] != 0; i++)
                    {
                        service.name[i] = msg.received.raw_buffer[i];
                    }
                    service.endpoint = msg.received.data[1].data;
                    service.major = msg.received.data[2].data;
                    service.minor = msg.received.data[3].data;

                    registered_services.push(service);


                    log::log$("(server) registered service: {} ({}.{}) at {}", service.name, service.major, service.minor, service.endpoint);
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
                    for(size_t j = 0; j < registered_services.len(); j++)
                    {
                        auto &service = registered_services[j];
                        bool name_match = true;
                        for(size_t i = 0; i < 80 && msg.received.raw_buffer[i] != 0; i++)
                        {
                            if(service.name[i] != msg.received.raw_buffer[i])
                            {
                                name_match = false;
                                break;
                            }
                        }
                        if(name_match && service.major == msg.received.data[1].data && service.minor >= msg.received.data[2].data)
                        {
                            resp.endpoint = service.endpoint;
                            resp.major = service.major;
                            resp.minor = service.minor;
                            break;
                        }    
                    }
                    log::log$("(server) get server request: {} ({}.{}) -> {}", resp.endpoint == 0 ? "not found" : "found", msg.received.data[1].data, msg.received.data[2].data, resp.endpoint);
                    IpcMessage reply = {};
                    reply.data[0].data = resp.endpoint;
                    reply.data[0].is_asset = false;
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