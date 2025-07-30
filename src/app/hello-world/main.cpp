#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"

int _main(mcx::MachineContext*)
{

    // attempt connection to server ID 0

    auto client = Wingos::Space::self().connect_to_ipc_server(0);

    
    client.wait_for_accept();
    
    log::log$("(client) connected to server with handle: {}", client.handle);
    

    // now do a call 
    IpcMessage message = {};
    message.data[0].data = 69420;
    message.data[0].is_asset = false;

    auto sended_message = client.send(message, true);
    auto message_handle = sended_message.unwrap();
    if(sended_message.is_error())
    {
        log::log$("no message handle returned");
    }
    else
    {
        log::log$("(client) message sent with handle: {}", message_handle);
    }

    
    while (true)
    {
        auto received = client.receive_reply(message_handle);
        if (!received.is_error())
        {
            log::log$("(client) received message: {}", received.unwrap().data[0].data);
            break;
        }
    }

    log::log$("dead...");
    while(true)
    {

    }
    return 1;
}