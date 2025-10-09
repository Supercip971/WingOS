#pragma once 
#include <stdint.h>
#include "iol/wingos/ipc.hpp"
#include "libcore/result.hpp"
#include "wingos-headers/ipc.h"
#include "iol/wingos/space.hpp"


namespace prot 
{


     enum InitMessageType 
    {
        INIT_REGISTER_SERVER = 1,
        INIT_UNREGISTER_SERVER = 2,
        INIT_GET_SERVER = 3,
        INIT_GET_SERVER_RESPONSE = 4,
    };
    struct InitRegisterServer 
    {
        // wingos/disk
        // init
    
        const char name[80]; 
        uint64_t major; 
        uint64_t minor;
        MessageHandle endpoint;   

    };

    struct InitUnregisterServer 
    {
        const char name[80]; 
    };

    struct InitGetServer 
    {
        const char name[80]; 
        uint64_t major;
        uint64_t minor;
    };

    struct InitGetServerResponse 
    {
        MessageHandle endpoint; 
        uint64_t major;
        uint64_t minor;
    };



    class InitConnection 
    {

        Wingos::IpcClient connection;
        
        public: 


        static core::Result<InitConnection> connect()
        {
            InitConnection conn {};
            conn.connection =  Wingos::Space::self().connect_to_ipc_server(0);

            conn.connection.wait_for_accept();

            return conn;

        }


        void end()
        {
            // FIXME: add a disconnect syscall
            //connection.();
        }
        core::Result<void> register_server(InitRegisterServer const &reg)
        {
            IpcMessage message = {};
            message.data[0].data = INIT_REGISTER_SERVER;
            message.data[1].data= reg.endpoint;
            message.data[2].data = reg.major;
            message.data[3].data = reg.minor;

            for (size_t i = 0; i < 80 && reg.name[i] != 0; i++)
            {
                message.raw_buffer[i] = reg.name[i];
            }

            auto sended_message = connection.send(message, false);
            auto message_handle = sended_message.unwrap();
            if (sended_message.is_error())
            {
                return ("failed to send register server message");
            }
            (void)message_handle;

            return {};

        }
        core::Result<void> unregister_server(InitUnregisterServer const &reg)
        {
            IpcMessage message = {};
            message.data[0].data = INIT_UNREGISTER_SERVER;
            for (size_t i = 0; i < 80 && reg.name[i] != 0; i++)
            {
                message.raw_buffer[i] = reg.name[i];
            }

            auto sended_message = connection.send(message, false);
            auto message_handle = sended_message.unwrap();
            if (sended_message.is_error())
            {
                return ("failed to send unregister server message");
            }
            (void)message_handle;

            return {};
        }
        core::Result<InitGetServerResponse> get_server(InitGetServer const &reg)
        {
            IpcMessage message = {};
            message.data[0].data = INIT_GET_SERVER;
            message.data[1].data = reg.major;
            message.data[2].data = reg.minor;

            for (size_t i = 0; i < 80 && reg.name[i] != 0; i++)
            {
                message.raw_buffer[i] = reg.name[i];
            }

            auto sended_message = connection.send(message, true);
            auto message_handle = sended_message.unwrap();
            if (sended_message.is_error())
            {
                return ("failed to send get server message");
            }

            while (true)
            {
                auto received = connection.receive_reply(message_handle);
                if (!received.is_error())
                {
                    auto msg = received.unwrap();
                    InitGetServerResponse resp {};
                    resp.endpoint = msg.data[0].data;
                    resp.major = msg.data[1].data;
                    resp.minor = msg.data[2].data;
                    return (resp);
                }
            }

            return ("failed to receive get server response");
        }
    };
} // namespace prot