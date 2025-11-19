#pragma once 

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/str.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/ipc.h"


namespace prot 
{
    enum CompositorMessageType 
    {
        COMPOSITOR_CREATE_WINDOW = 1,
    };


    struct CompositorCreateWindow 
    {
        IpcServerHandle window_endpoint;
    };


    class CompositorConnection 
    {
        Wingos::IpcClient connection;
    public:
        Wingos::IpcClient &raw_client() { return connection; }
        static core::Result<CompositorConnection> connect()
        {
            CompositorConnection conn {};

            auto reg = try$(InitConnection::connect());

            auto handle = try$(reg.get_server(core::Str("compositor"), 1, 0)).endpoint;
            conn.connection =  Wingos::Space::self().connect_to_ipc_server(handle, 1, 0);

            conn.connection.wait_for_accept();

            return conn;
        }

        IpcServerHandle create_window(bool take_fb = false)
        {
            IpcMessage message = {};
            message.data[0].data = COMPOSITOR_CREATE_WINDOW;
            message.data[1].data = take_fb ? 1 : 0;

            auto sended_message = connection.call(message);
            if (sended_message.is_error())
            {
                log::err$("compositor: failed to send create window message");
            }

            auto msg = core::move(sended_message.unwrap());
            IpcServerHandle window_endpoint = msg.data[0].data;

            log::log$("compositor: created window with endpoint {}", window_endpoint);
            return window_endpoint;
        }
    };

};