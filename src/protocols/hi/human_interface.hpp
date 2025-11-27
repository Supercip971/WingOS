#pragma once 

#include <stdint.h>
#include "iol/wingos/ipc.hpp"
#include "protocols/init/init.hpp"

namespace prot 
{

    enum EventType : uint32_t
    {
        HI_EVENT_NOEVENT = 0,
        HI_EVENT_TYPE_MOUSE = 1,
        HI_EVENT_TYPE_KEYBOARD = 2,
    };

    enum HIMessageType : uint32_t
    {
        HI_START_LISTEN = 0,
        
    };
    struct HIEvent 
    {
        EventType type;
        union    
        {
            struct 
            {
                int32_t dx;
                int32_t dy;
                uint8_t buttons;
            } mouse;
            struct 
            {
                uint32_t keycode;
                bool pressed;
            } keyboard;
        };
    }   ;

    class HIConnection 
    {
        Wingos::IpcClient connection;

    public:
        Wingos::IpcClient &raw_client() { return connection; }

        core::Result<void> start_listen()
        {
            IpcMessage message = {};
            message.data[0].data = HI_START_LISTEN; 

            auto sended_message = connection.send(message, false);

            if (sended_message.is_error())
            {
                return "failed to send HI start listen message";
            }

            return {};
        }

        static core::Result<HIConnection> connect()
        {
            HIConnection hi_conn;
            auto reg = InitConnection::connect();
            if (reg.is_error())
            {
                return core::Result<HIConnection>::error("failed to connect to init");
            }
            auto v = reg.unwrap();
            auto handle = try$(v.get_server(core::Str("human-interface"), 1, 0)).endpoint;
            hi_conn.connection = Wingos::Space::self().connect_to_ipc_server(handle);
            hi_conn.connection.wait_for_accept();
            return hi_conn;
        }
        core::Result<HIEvent> receive_event()
        {
            auto msg = connection.receive();
            if (msg.is_error())
            {
                return core::Result<HIEvent>::error("failed to receive HI event");
            }
            HIEvent event = {};

            auto& dat = msg.unwrap().received.data;
            event.type = static_cast<EventType>(dat[0].data);
            switch (event.type)
            {
            case HI_EVENT_TYPE_MOUSE:
                event.mouse.dx = static_cast<int32_t>(dat[1].data);
                event.mouse.dy = static_cast<int32_t>(dat[2].data);
                event.mouse.buttons = static_cast<uint8_t>(dat[3].data);
                break;
            case HI_EVENT_TYPE_KEYBOARD:
                event.keyboard.keycode = static_cast<uint32_t>(dat[1].data);
                event.keyboard.pressed = static_cast<bool>(dat[2].data);
                break;
            case HI_EVENT_NOEVENT:
                break;
            default:
                return core::Result<HIEvent>::error("invalid HI event type");
            }
            return event;
        }

    };
}