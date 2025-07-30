#include "arch/generic/syscalls.h"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"

int _main(mcx::MachineContext*)
{

    // attempt connection to server ID 0

    SyscallIpcConnect connect = sys$ipc_connect(0, SPACE_SELF, 0, 0);


    (void)connect;

    while(true)
    {
        auto res = sys$ipc_status(0, connect.returned_handle);
        if(res.returned_is_accepted)
        {
            log::log$("connection is accepted: {}", connect.returned_handle);
            break;
        }
        else
        {
         //   log::log$("connection is not accepted yet, retrying...");
        }
    }
    if(connect.returned_handle == 0)
    {
        log::log$("no connection available");
    }
    else
    {
        log::log$("connected to server with handle: {}", connect.returned_handle);
    }

    // now do a call 
    IpcMessage message = {};
    message.data[0].data = 69420;
    message.data[0].is_asset = false;

    SyscallIpcSend call = sys$ipc_send(0, connect.returned_handle, message, true);
    if(call.returned_msg_handle == 0)
    {
        log::log$("no message handle returned");
    }
    else
    {
        log::log$("message sent with handle: {}", call.returned_msg_handle);
    }

    
    while (true)
    {
        
        auto res = sys$ipc_receive_reply_client(false, SPACE_SELF, connect.returned_handle, call.returned_msg_handle);
        if (res.contain_response)
        {
            log::log$("received message: {}", res.returned_message.data[0].data);
            break;
        }
    }

    log::log$("dead...");
    while(true)
    {

    }
    return 1;
}