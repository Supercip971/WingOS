#pragma once
#include "protocols/hi/human_interface.hpp"
#include "protocols/server_helper.hpp"

#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/optional.hpp"
#include "libcore/result.hpp"
#include "protocols/pipe/pipe.hpp"
#include "wingos-headers/ipc.h"

class Ps2Ctx
{
};

class Ps2Connection : public prot::ManagedServerConnectionHandler
{
public:
    prot::Duplex<prot::HIEvent> pipe;
    Wingos::SignalAsset *acc_mouse_signal;
    Wingos::SignalAsset *acc_kb_signal;
    uint32_t event_types = 0;
    Ps2Connection() = default;

    virtual bool init() { return true; }

    virtual void signal_disconnect(IpcMessage &msg)
    {
        (void)msg;
    };

    virtual ~Ps2Connection() = default;

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
        case prot::HI_ENABLE_SIGNALING:
        {
            IpcMessage resp = {};

            if (msg.arg(1) == prot::HI_EVENT_TYPE_MOUSE)
            {
                resp.copy_handle(0, acc_mouse_signal->handle);
            }
            else if (msg.arg(1) == prot::HI_EVENT_TYPE_KEYBOARD)
            {
                resp.copy_handle(0, acc_kb_signal->handle);
            }
            else
            {
                fmt::warn$("invalid HI_ENABLE_SIGNALING kind: {}", msg.arg(1));
            }

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

    Wingos::SignalAsset *acc_mouse_signal;
    Wingos::SignalAsset *acc_kb_signal;

public:
    void init(Wingos::SignalAsset *mouse, Wingos::SignalAsset *kb)
    {
        acc_mouse_signal = mouse;
        acc_kb_signal = kb;
    }

    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        (void)initiator;
        auto res = new Ps2Connection;
        res->acc_kb_signal = acc_kb_signal;
        res->acc_mouse_signal = acc_mouse_signal;
        return res;
    };

    virtual ~Ps2Server() = default;
};
