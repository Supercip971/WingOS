#pragma once 

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/str.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/ipc.h"


namespace prot 
{
    enum ClockMessageType
    {
        CLOCK_GET_SYSTEM_TIME = 1,
        CLOCK_SLEEP_MS = 2,
    };


    struct ClockTime 
    {
        uint64_t seconds;
        uint64_t milliseconds;
    };
    
    class ClockConnection 
    {
        Wingos::IpcClient connection;
    public:
        Wingos::IpcClient &raw_client() { return connection; }
        static core::Result<ClockConnection> connect()
        {
            ClockConnection conn {};

            auto reg = try$(InitConnection::connect());

            auto handle = try$(reg.get_server(core::Str("clock"), 1, 0)).endpoint;
            conn.connection =  Wingos::Space::self().connect_to_ipc_server(handle, 1, 0);

            conn.connection.wait_for_accept();

            return conn;
        }

        core::Result<ClockTime> get_system_time()
        {
            IpcMessage message = {};
            message.data[0].data = CLOCK_GET_SYSTEM_TIME;

            auto sended_message = connection.call(message);
            if (sended_message.is_error())
            {
                return ("clock: failed to send get system time message");
            }

            auto msg = core::move(sended_message.unwrap());
            ClockTime time {};
            time.seconds = msg.data[1].data;
            time.milliseconds = msg.data[2].data;

            return time;
        }
        

        core::Result<void> sleep_ms(uint64_t ms)
        {
            IpcMessage message = {};
            message.data[0].data = CLOCK_SLEEP_MS;
            message.data[1].data = ms;

            auto sended_message = connection.call(message);
            if (sended_message.is_error())
            {
                return ("clock: failed to send sleep ms message");
            }

            return {};
        }
   };

}