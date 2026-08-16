#pragma once

#include "protocols/server_helper.hpp"

#include "hw/hpet/hpet.hpp"
#include "iol/wingos/ipc.hpp"
#include "libcore/time/time.hpp"
#include "protocols/clock/clock.hpp"
#include "wingos-headers/ipc.h"

class HPetCtx
{
};

class HPetConnection : public prot::ManagedServerConnectionHandler
{
public:
    bool waiting = false;
    fc::Milliseconds start_time;
    fc::Milliseconds end_time;
    Wingos::IpcReplyObject waiter;
    HPetConnection() = default;

    virtual bool init() { return true; }

    virtual void signal_disconnect(IpcMessage &msg)
    {
        (void)msg;
    };

    virtual ~HPetConnection()
    {
    }

    virtual fc::Result<void> call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj)
    {
        switch (msg.arg(0))
        {
        case prot::CLOCK_GET_SYSTEM_TIME:
        {

            IpcMessage reply = {};
            fc::Milliseconds ms = hw::hpet::hpet_clock_read();
            reply.arg(0, ms.value() / 1000);
            reply.arg(1, ms.value());

            ret(reply);
            break;
        }
        case prot::CLOCK_SLEEP_MS:
        {

            if (!reply_obj.has_value())
                return "sleeping without using call()";
            this->waiting = true;
            this->waiter = reply_obj.value();
            this->start_time = hw::hpet::hpet_clock_read();
            this->end_time = this->start_time + fc::Milliseconds(msg.arg(1));
            break;
        }
        default:
        {
            fmt::warn$("unknown command: {}", msg.arg(0));
            return "unknown command";
        }
        }
        return {};
    }
};

class HpetServer : public prot::ManagedServer

{
public:
    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        (void)initiator;
        return new HPetConnection;
    };

    virtual ~HpetServer()
    {
    }
};
