#pragma once
#include "protocols/hi/human_interface.hpp"
#include "protocols/server_helper.hpp"

#include "iol/wingos/ipc.hpp"
#include "libcore/optional.hpp"
#include "protocols/pipe/pipe.hpp"
#include "wingos-headers/ipc.h"

class Ps2Ctx
{
};

class Ps2Connection : public prot::ManagedServerConnectionHandler
{
public:
    prot::Duplex<prot::HIEvent> pipe;
    uint32_t event_types = 0;
    Ps2Connection() = default;

    virtual bool init() { return true; }

    virtual void signal_disconnect(IpcMessage &msg)
    {
        (void)msg;
    };

    virtual ~Ps2Connection()
    {
    }

    virtual fc::Result<void> call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj [[maybe_unused]])
    {
        switch (msg.arg(0))
        {
        case prot::HI_START_LISTEN:
        {
            event_types = (uint32_t)msg.arg(1);

            pipe = try$(prot::Duplex<prot::HIEvent>::create(Wingos::Space::self()));

            if (event_types & prot::HI_EVENT_TYPE_MOUSE)
            {
                fmt::log$("hio: added mouse pipe");
            }

            if (event_types & prot::HI_EVENT_TYPE_KEYBOARD)
            {
                fmt::log$("hio: added keyboard pipe");
            }

            IpcMessage resp = {};
            resp.copy_handle(0, pipe.ring_phys_asset.handle);
            ret(resp);
            break;
        }
        default:
        {
            fmt::warn$("hio: unknown message type received: {}", msg.arg(0));
            break;
        }
        }

        return {};
    }
};

class Ps2Server : public prot::ManagedServer

{
public:
    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        (void)initiator;
        return new Ps2Connection;
    };

    virtual ~Ps2Server()
    {
    }
};
