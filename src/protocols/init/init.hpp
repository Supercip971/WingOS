#pragma once
#include <stdint.h>

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "wingos-headers/ipc.h"

namespace prot
{

enum InitMessageType
{
    INIT_REGISTER_SERVER = 1,
    INIT_UNREGISTER_SERVER = 2,
    INIT_GET_SERVER = 3,
    INIT_GET_SERVER_RESPONSE = 4,
    INIT_SIGNAL_FS_AVAILABLE = 5,
    INIT_QUERY_FB = 6,
};

struct InitRegisterServer
{
    // wingos/disk
    // init

    char name[80];
    uint64_t major;
    uint64_t minor;
    MessageHandle endpoint;
};

struct InitUnregisterServer
{
    char name[80];
};

struct InitGetServer
{
    char name[80];
    uint64_t major;
    uint64_t minor;
};

struct InitGetServerResponse
{
    IpcServerHandle endpoint;
    uint64_t major;
    uint64_t minor;
};

struct InitQueryFbResponse
{
    uintptr_t framebuffer_addr;
    size_t framebuffer_width;
    size_t framebuffer_height;
};

class InitConnection
{

    Wingos::IpcClient connection;

public:
    Wingos::IpcClient &raw_client() { return connection; }

    static fc::Result<InitConnection> connect()
    {
        InitConnection conn{};
        conn.connection = Wingos::Space::self().connect_by_addr(0);
        return conn;
    }

    void end()
    {
        connection.disconnect();
    }

    fc::Result<void> register_server(InitRegisterServer const &reg)
    {
        IpcMessage message = {};
        message.arguments.data[0].data = INIT_REGISTER_SERVER;
        message.arguments.data[1].data = reg.endpoint;
        message.arguments.data[2].data = reg.major;
        message.arguments.data[3].data = reg.minor;
        size_t i;

        for (i = 0; i < 80 && reg.name[i] != 0; i++)
        {
            message.raw_buffer[i] = reg.name[i];
        }

        message.raw_buffer[i] = 0;

        message.len = i + 1;

        fmt::log$("Register server {START} ");

        auto sended_message = connection.send(message);

        fmt::log$("Register server {END } ");

        return {};
    }

    fc::Result<void> unregister_server(InitUnregisterServer const &reg)
    {
        IpcMessage message = {};
        message.arguments.data[0].data = INIT_UNREGISTER_SERVER;
        for (size_t i = 0; i < 80 && reg.name[i] != 0; i++)
        {
            message.raw_buffer[i] = reg.name[i];
        }

        connection.send(message);
        return {};
    }

    fc::Result<InitGetServerResponse> get_server(InitGetServer const &reg)
    {
        IpcMessage message = {};
        message.arguments.data[0].data = INIT_GET_SERVER;
        message.arguments.data[1].data = reg.major;
        message.arguments.data[2].data = reg.minor;
        size_t i;
        for (i = 0; i < 80 && reg.name[i] != 0; i++)
        {
            message.raw_buffer[i] = reg.name[i];
        }

        message.raw_buffer[i] = 0;
        message.len = i + 1;

        fmt::log$("Querying server {START} ");

        auto res = connection.call(message);

        fmt::log$("Querying server {END  } ");
        if (!res.is_error())
        {
            InitGetServerResponse resp{};
            resp.endpoint = message.arguments.data[0].data;
            if (resp.endpoint == 0)
            {
                return ("server not found");
            }
            resp.major = message.arguments.data[1].data;
            resp.minor = message.arguments.data[2].data;
            fmt::log$("got server response: endpoint={}, major={}, minor={}", resp.endpoint, resp.major, resp.minor);
            return (resp);
        }
        fmt::log$("failed to get server response");

        return ("failed to receive get server response");
    }

    fc::Result<InitGetServerResponse> get_server(fc::Str name, uint64_t major, uint64_t minor)
    {
        InitGetServer get = {

            .name = {},
            .major = major,
            .minor = minor,
        };

        name.copy_to((char *)get.name, 80);

        return get_server(get);
    }

    fc::Result<void> signal_fs_available()
    {
        IpcMessage message = {};
        message.arguments.data[0].data = INIT_SIGNAL_FS_AVAILABLE;

        connection.send(message);

        return {};
    }

    fc::Result<InitQueryFbResponse> query_framebuffer()
    {
        IpcMessage message = {};
        message.arguments.data[0].data = INIT_QUERY_FB;

        auto res = connection.call(message);
        if (!res.is_error())
        {
            InitQueryFbResponse resp{};
            resp.framebuffer_addr = message.arguments.data[0].data;
            resp.framebuffer_width = message.arguments.data[1].data;
            resp.framebuffer_height = message.arguments.data[2].data;
            return (resp);
        }
        return ("failed to receive framebuffer info");
    }
};
} // namespace prot
