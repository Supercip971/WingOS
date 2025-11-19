#pragma once 
#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "protocols/compositor/compositor.hpp"


namespace prot 
{
    enum WindowMessageType 
    {
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
        Wingos::IpcClient connection;

        Wingos::MemoryAsset mem_asset = {};
        Wingos::VirtualMemoryAsset virt_asset = {};
    public:
        Wingos::IpcClient &raw_client() { return connection; }

        static core::Result<WindowConnection> create(bool take_fb = false)
        {
            WindowConnection conn {};
            auto comp = try$(CompositorConnection::connect());

            auto window_endpoint = comp.create_window(take_fb);
            conn.connection =  Wingos::Space::self().connect_to_ipc_server(window_endpoint);
            conn.connection.wait_for_accept();

            return conn;
        }

        core::Result<WindowGetAttributeSize> get_attribute_size()
        {
            IpcMessage message = {};
            message.data[0].data = WINDOW_GET_ATTRIBUTE_SIZE;

            auto res = connection.call(message);
            if (!res.is_error())
            {
                auto msg = res.take();
                WindowGetAttributeSize resp {};
                resp.width = msg.data[0].data;
                resp.height = msg.data[1].data;
                return (resp);
            }
            return ("failed to receive attribute size");
        }

        core::Result<Wingos::VirtualMemoryAsset> get_framebuffer()
        {
            IpcMessage message = {};
            message.data[0].data = WINDOW_GET_FRAMEBUFFER;

            auto res = connection.call(message);
            if (!res.is_error())
            {
                auto msg = res.take();
                mem_asset.handle = msg.data[0].data;

                mem_asset = Wingos::MemoryAsset::from_handle(mem_asset.handle);
                virt_asset = Wingos::Space::self().map_memory(mem_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
                
                return (virt_asset);
            }
            return ("failed to receive framebuffer asset");
        }   

        core::Result<void> swap_buffers()
        {
            IpcMessage message = {};
            message.data[0].data = WINDOW_SWAP_BUFFERS;

            auto sended_message = connection.send(message, false);
            return {};
        }
    };
};