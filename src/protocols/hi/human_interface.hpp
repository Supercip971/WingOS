#pragma once

#include <stdint.h>

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "protocols/init/init.hpp"
#include "protocols/pipe/pipe.hpp"

namespace prot
{

enum EventType : uint32_t
{
    HI_EVENT_NOEVENT = 0,
    HI_EVENT_TYPE_MOUSE = 1 << 1,
    HI_EVENT_TYPE_KEYBOARD = 1 << 2,
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
};

class HIEventQueue
{
    ReceiverPipe _rpipe = {};
    core::Vec<HIEvent> _events = {};

public:
    HIEventQueue() = default;
    HIEventQueue(ReceiverPipe &&rpipe) : _rpipe(core::move(rpipe))
    {
    }
    void push_event(const HIEvent &event)
    {
        _events.push(event);
    }
    core::Result<HIEvent> poll_event()
    {
        if (_events.len() == 0)
        {
            return core::Result<HIEvent>::error("no event");
        }
        return _events.pop(0);
    }

    void update_event()
    {
        HIEvent event;
        auto res = _rpipe.receive(&event, sizeof(HIEvent));
        while (!res.is_error())
        {

            push_event(event);
            res = _rpipe.receive(&event, sizeof(HIEvent));
        }
    }
};
class HIConnection
{
    Wingos::IpcClient connection;
    HIEventQueue _event_queue;

public:
    Wingos::IpcClient &raw_client() { return connection; }

    HIEventQueue &event_queue() { return _event_queue; }

    core::Result<void> start_listen()
    {
        IpcMessage message = {};
        message.data[0].data = HI_START_LISTEN;
        message.data[1].data = HI_EVENT_TYPE_MOUSE | HI_EVENT_TYPE_KEYBOARD;

        auto res = connection.call(message);

        if (res.is_error())
        {
            return "failed to send HI start listen message";
        }

        auto msg = core::move(res.unwrap());

        auto rpipe = try$(ReceiverPipe::from(Wingos::Space::self(), msg.data[0].asset_handle));
        _event_queue = HIEventQueue(core::move(rpipe));

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
};
} // namespace prot