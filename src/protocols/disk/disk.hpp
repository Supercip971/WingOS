#pragma once

#include "iol/wingos/ipc.hpp"
#include "libcore/str.hpp"
#include "protocols/init/init.hpp"

namespace prot
{
enum DiskMessageType
{
    DISK_IDENTIFY_DEVICE = 0,
    DISK_READ_SECTORS = 1,
    DISK_WRITE_SECTORS = 2,
    DISK_READ_SMALL = 3,
    DISK_WRITE_SMALL = 4,
};

class DiskConnection
{

    Wingos::IpcClient connection;

public:
    Wingos::IpcClient &raw_client() { return connection; }

    core::Result<size_t> read_small(void *buffer, uint64_t lba, uint64_t len)
    {
        if (len > MAX_IPC_BUFFER_SIZE)
        {
            return "length exceeds max ipc buffer size";
        }

        IpcMessage message = {};
        message.data[0].data = DISK_READ_SMALL;
        message.data[1].data = lba;
        message.data[2].data = len;
        auto sended_message = connection.send(message);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send read small sectors message");
        }
        while (true)
        {
            auto received = connection.receive_reply(message_handle);
            if (!received.is_error())
            {
                auto msg = received.unwrap();
                for (size_t i = 0; i < len; i++)
                {
                    ((uint8_t *)buffer)[i] = msg.raw_buffer[i];
                }
                return len;
            }
        }
    }
    core::Result<void> read(Wingos::MemoryAsset &asset, uint64_t lba, uint64_t len)
    {
        if (len < MAX_IPC_BUFFER_SIZE)
        {
            log::warn$("read length is smaller than max ipc buffer size, consider using read_small");
        }

        if (len % 512 != 0)
        {
            return "length must be multiple of 512 bytes";
        }

        IpcMessage message = {};
        message.data[0].data = DISK_READ_SECTORS;
        message.data[1].data = lba;
        message.data[2].data = len;
        message.data[3].is_asset = true;
        message.data[3].asset_handle = asset.handle;
        auto sended_message = connection.send(message, true);
        auto message_handle = sended_message.unwrap();

        if (sended_message.is_error())
        {
            return ("failed to send read sectors message");
        }
        while (true)
        {

            auto received = connection.receive_reply(message_handle);
            if (!received.is_error())
            {
                auto msg = received.unwrap();
                asset = Wingos::MemoryAsset::from_handle(msg.data[1].asset_handle);

                return {};

            }
        }
        // swap back
               (void)message_handle;
        return {};
    }

    core::Result<void> write_small(void *buffer, uint64_t lba, uint64_t len)
    {
        if (len > MAX_IPC_BUFFER_SIZE)
        {
            return "length exceeds max ipc buffer size";
        }

        IpcMessage message = {};
        message.data[0].data = DISK_WRITE_SMALL;
        message.data[1].data = lba;
        message.data[2].data = len;
        for (size_t i = 0; i < len; i++)
        {
            message.raw_buffer[i] = ((uint8_t *)buffer)[i];
        }
        message.len = len;
        auto sended_message = connection.send(message, false);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send write small sectors message");
        }

        (void)message_handle;
        return {};
    }

    core::Result<void> write(Wingos::MemoryAsset &asset, uint64_t lba, uint64_t len)
    {
        if (len < MAX_IPC_BUFFER_SIZE)
        {
            log::warn$("write length is smaller than max ipc buffer size, consider using write_small");
        }

        IpcMessage message = {};
        message.data[0].data = DISK_WRITE_SECTORS;
        message.data[1].data = lba;
        message.data[2].data = len;
        message.data[3].is_asset = true;
        message.data[3].asset_handle = asset.handle;
        auto sended_message = connection.send(message, false);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send write sectors message");
        }

        (void)message_handle;
        (void)sended_message;
        return {};
    }

    static core::Result<DiskConnection> connect(core::Str dev_name)
    {
        DiskConnection conn;
        auto init_conn = try$(prot::InitConnection::connect());

        auto v = try$(init_conn.get_server(dev_name, 1, 0));

        conn.connection = Wingos::Space::self().connect_to_ipc_server(v.endpoint);

        conn.connection.wait_for_accept();
        log::log$("Connected to disk server at address: {} ({})", v.endpoint, conn.connection.associated_space_handle);
        return conn;
    }
};
} // namespace prot