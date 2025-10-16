#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "protocols/vfs/vfs.hpp"
#include "protocols/init/init.hpp"


#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"



struct RegisteredDevicePartition 
{
    uint64_t id;
    IpcServerHandle endpoint;
    char name[80];
};

struct RegisteredDevice {
    char name[80];
    IpcServerHandle endpoint;   
    bool has_partitions;
    core::Vec<RegisteredDevicePartition> partitions;
};

int _main(mcx::MachineContext* )
{

    // attempt connection to server ID 0
  

    auto iconn = prot::InitConnection::connect().unwrap();
 

    prot::InitRegisterServer reg = {};
    auto server= Wingos::Space::self().create_ipc_server(false);
    core::Str("vfs").copy_to((char*)reg.name, 80);

    reg.major = 1;
    reg.minor = 0;
    reg.endpoint = server.addr;

    iconn.register_server(reg).unwrap();

    core::Vec<Wingos::IpcConnection *> connections;
    core::Vec<RegisteredDevice> registered_services;


    while(true){ 
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
            
            bool check_partition = false;
            switch(msg.received.data[0].data)
            {
                case prot::VFS_REGISTER:
                {
                    RegisteredDevice device {};
                    for(size_t i = 0; i < 80 && msg.received.raw_buffer[i] != 0; i++)
                    {
                        device.name[i] = msg.received.raw_buffer[i];
                    }
                    device.endpoint = msg.received.data[1].data;
                    device.has_partitions = false;

                    registered_services.push(device);
                    check_partition = true;

                    log::log$("(server) registered device: {} with endpoint: {}", device.name, device.endpoint);
                    break;
                }
                default:
                {
                    log::log$("(server) unknown message type: {}", msg.received.data[0].data);
                    break;
                }
            }

            if(check_partition)
            {
                log::log$("(server) checking partitions for device: {}", registered_services[registered_services.len() - 1].name);
            }
        }
    }

}