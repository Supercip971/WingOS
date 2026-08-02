#pragma once

#include <stdlib.h>
#include <string.h>

#include "app/init/module_startup.hpp"
#include "protocols/server_helper.hpp"

#include "iol/wingos/ipc.hpp"
#include "libcore/optional.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "wingos-headers/ipc.h"

struct MachineContextShared
{
    uintptr_t framebuffer_addr;
    size_t framebuffer_width;
    size_t framebuffer_height;
};

fc::Result<IpcServerHandle> service_get(fc::Str const &name, uint64_t major = 1, uint64_t minor = 0);

fc::Result<void> service_register(uint64_t endpoint, fc::Str const &name, uint64_t major = 1, uint64_t minor = 0);

class InitIpcConnection : public prot::ManagedServerConnectionHandler
{
    MachineContextShared *shared;

public:
    InitIpcConnection() {};

    bool init() final
    {
        return true;
    }

    virtual bool call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj)
    {
        switch (msg.arg(0))
        {
        case prot::INIT_REGISTER_SERVER:
        {
            fmt::log$("registered server: {}");
            service_register(
                msg.arg(1),
                fc::Str((char *)msg.raw_buffer, msg.len - 1),
                msg.arg(2),
                msg.arg(3))
                .assert();
            break;
        }
        case prot::INIT_UNREGISTER_SERVER:
        {
            fmt::warn$("(server) unregister server not implemented yet");
            break;
        }
        case prot::INIT_GET_SERVER:
        {
            prot::InitGetServerResponse resp{};
            resp.endpoint = -1;

            auto name_len = msg.len - 1;
            fc::Str name = fc::Str((char *)msg.raw_buffer, name_len);

            auto service_res = service_get(
                name,
                msg.arg(1),
                msg.arg(2));

            if (!service_res.is_error())
            {
                resp.endpoint = service_res.unwrap();
            }
            else
            {
                fmt::log$("(server) get server failed: {}", service_res.error());
            }

            IpcMessage ret = {};
            ret.arguments.data[0].data = resp.endpoint;
            ret.arguments.data[0].is_asset = false;

            reply(ret, reply_obj.unwrap());
            break;
        }
        case prot::INIT_SIGNAL_FS_AVAILABLE:
        {
            fmt::log$("(server) received signal fs available");
            service_startup_callback("@fs");
            break;
        }

        case prot::INIT_QUERY_FB:
        {
            prot::InitQueryFbResponse resp{};
            resp.framebuffer_addr = shared->framebuffer_addr;
            resp.framebuffer_width = shared->framebuffer_width;
            resp.framebuffer_height = shared->framebuffer_height;

            IpcMessage ret = {};
            ret.arg(0, resp.framebuffer_addr);
            ret.arg(1, resp.framebuffer_width);
            ret.arg(2, resp.framebuffer_height);
            reply(ret, reply_obj).assert();
            break;
        }
        default:
        {
            fmt::log$("(server) unknown message type: {}", msg.arg(0));
            break;
        }
        }
        return {};
    }
};

class InitIpcServer : public prot::ManagedServer
{
    MachineContextShared shared_ctx;

public:
    MachineContextShared const &mcx_shared() { return shared_ctx; }

    void mcx_shared(MachineContextShared const &val) { shared_ctx = val; }

    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        (void)initiator;
        return {new InitIpcConnection{}};
    }

    virtual ~InitIpcServer() = default;
};
