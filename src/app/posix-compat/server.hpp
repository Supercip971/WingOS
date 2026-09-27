#pragma once

#include <stddef.h>
#include <string.h>

#include "protocols/server_helper.hpp"

#include "iol/wingos/asset.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/humming-bird/humming-bird.hpp"
#include "protocols/init/init.hpp"

extern pid_t posix_pid_counter;

struct PosixProcess : public prot::ManagedServerConnectionHandler
{
    pid_t pid = 1;

    virtual bool init()
    {
        pid = posix_pid_counter++;
        return true;
    }

    virtual void signal_disconnect(IpcMessage &msg)
    {
        (void)msg;
    };

    // return true on reply
    virtual fc::Result<void> call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj [[maybe_unused]])
    {

        switch (msg.arg(0))
        {
        case prot::HB_POSIX_GETPID:
        {
            IpcMessage reply = {};
            reply.arg(0, pid);
            ret(reply);
            break;
        }
        case prot::HB_POSIX_REGISTER_SELF:
        {
            ack(reply_obj);
            break;
        }

        default:
        {
            fmt::warn$("compositor: unknown window message type received: {}", msg.arg(0));
            break;
        }
        }

        return {};
    }
};

class PosixServer : public prot::ManagedServer
{

public:
    PosixServer()
    {
    }

    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        switch (initiator.arg(0))
        {

        case prot::HB_POSIX_REGISTER_SELF:
        {
            auto conn = new PosixProcess();
            return conn;

            // case prot::VFS_ACCESS_PWD:
            // unimplemented
        }

        default:
            fmt::log$("invalid connect access for compositor: {}", initiator.arg(0));
            return "invalid connect access for compositor";
        }

        return "invalid connect access for compositor";
    }

    virtual fc::Result<void> after_receive() final
    {
        return {};
    }

    virtual ~PosixServer() {}
};
