#pragma once
#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/result.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

namespace prot
{
enum WindowMessageType
{
    COMPOSITOR_CREATE_WINDOW,
    WINDOW_GET_ATTRIBUTE_SIZE,
    WINDOW_GET_FRAMEBUFFER,
    WINDOW_SWAP_BUFFERS,
};

struct WindowGetAttributeSize
{
    uint64_t width;
    uint64_t height;
};

class WindowConnection
{
    bool has_swap;
    Wingos::IpcClient connection;

    Wingos::MemoryAsset mem_asset = {};
    Wingos::VirtualMemoryAsset virt_asset = {};

public:
    Wingos::IpcClient &raw_client() { return connection; }

    static fc::Result<WindowConnection> connect()
    {
        WindowConnection conn{};

        auto reg = try$(InitConnection::connect());

        auto handle = try$(reg.get_server(fc::Str("compositor"), 1, 0)).endpoint;
        conn.connection = Wingos::Space::self().connect_by_addr(handle);

        reg.end();

        return conn;
    }

    IpcServerHandle create_window(bool take_fb = false)
    {
        IpcMessage message = {};
        message.arguments.data[0].data = COMPOSITOR_CREATE_WINDOW;
        message.arguments.data[1].data = take_fb ? 1 : 0;

        auto sended_message = connection.call(message);
        if (sended_message.is_error())
        {
            fmt::err$("compositor: failed to send create window message");
        }

        fmt::log$("compositor: created window with endpoint {}", connection.handle);
        return connection.handle;
    }

    static fc::Result<WindowConnection> create(bool take_fb = false)
    {
        auto comp = try$(WindowConnection::connect());

        comp.create_window(take_fb);

        return comp;
    }

    fc::Result<WindowGetAttributeSize> get_attribute_size()
    {
        IpcMessage message = {};
        message.arguments.data[0].data = WINDOW_GET_ATTRIBUTE_SIZE;

        auto res = connection.call(message);
        if (!res.is_error())
        {
            WindowGetAttributeSize resp{};
            resp.width = message.arguments.data[0].data;
            resp.height = message.arguments.data[1].data;
            return (resp);
        }
        return ("failed to receive attribute size");
    }

    fc::Result<Wingos::VirtualMemoryAsset> get_framebuffer()
    {
        IpcMessage message = {};
        message.arguments.data[0].data = WINDOW_GET_FRAMEBUFFER;

        auto res = connection.call(message);
        if (!res.is_error())
        {
            mem_asset.handle = message.asset(0);

            mem_asset = Wingos::MemoryAsset::from_handle(mem_asset.handle);
            virt_asset = Wingos::Space::self().map_memory(mem_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

            return (virt_asset);
        }
        return ("failed to receive framebuffer asset");
    }

    fc::Result<void> swap_buffers()
    {

        IpcMessage message = {};
        message.arguments.data[0].data = WINDOW_SWAP_BUFFERS;

        auto sended_message = connection.send(message);

        has_swap = true;
        return {};
    }
};
}; // namespace prot
