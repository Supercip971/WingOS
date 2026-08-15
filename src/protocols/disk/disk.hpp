#pragma once

#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "math/align.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

namespace prot
{
enum DiskMessageType
{
    DISK_IDENTIFY_DEVICE = 0,
    DISK_READ_SECTORS = 1,
    DISK_WRITE_SECTORS = 2,
    DISK_READ_SMALL = 3,
    DISK_WRITE_SMALL = 4,
    DISK_FORK_CONNECTION = 5,
};

class DiskConnection
{

    Wingos::IpcClient connection;

public:
    Wingos::IpcClient &raw_client() { return connection; }

    fc::Result<DiskConnection> fork_client()
    {
        IpcMessage message = {};
        message.arg(0, DISK_FORK_CONNECTION);
        connection.call(message);

        return DiskConnection::use_asset(message.asset(0));
    }

    fc::Result<size_t> read_small(void *buffer, uint64_t lba, uint64_t len)
    {
        if (len > MAX_IPC_BUFFER_SIZE)
        {
            return "length exceeds max ipc buffer size";
        }

        IpcMessage message = {};
        message.arguments.data[0].data = DISK_READ_SMALL;
        message.arguments.data[1].data = lba;
        message.arguments.data[2].data = len;
        auto sended_message = connection.call(message);
        for (size_t i = 0; i < len; i++)
        {
            ((uint8_t *)buffer)[i] = message.raw_buffer[i];
        }
        return len;
    }

    fc::Result<size_t> read(Wingos::MemoryAsset &asset, uint64_t lba, uint64_t len, uint64_t asset_start = 0)
    {
        if (len < MAX_IPC_BUFFER_SIZE)
        {
            fmt::warn$("read length is smaller than max ipc buffer size, consider using read_small");
        }

        if (len % 512 != 0)
        {
            return "length must be multiple of 512 bytes";
        }

        IpcMessage message = {};
        message.arguments.data[0].data = DISK_READ_SECTORS;
        message.arguments.data[1].data = lba;
        message.arguments.data[2].data = len;

        message.arguments.data[3].is_asset = true;
        message.arguments.data[3].asset_handle = asset.handle;
        message.arguments.data[4].data = asset_start;
        auto sended_message = connection.call(message);
        size_t bytes_read = message.arguments.data[0].data;
        if (len > 0 && bytes_read == 0)
        {
            return "disk read returned no data";
        }
        if (message.arguments.data[1].is_asset)
        {
            asset = Wingos::MemoryAsset::from_handle(message.arguments.data[1].asset_handle);
        }
        return bytes_read;
        // swap back
    }

    fc::Result<Wingos::VirtualMemoryAsset> read_buf(uint64_t lba, uint64_t count)
    {

        Wingos::MemoryAsset asset = Wingos::Space::self().allocate_physical_memory(math::alignUp<size_t>(count, 4096), false);

        size_t aligned_len = math::alignUp<size_t>(count, 512);
        auto read_res = read(asset, lba, aligned_len);

        if (read_res.is_error())
        {
            Wingos::Space::self().release_asset(asset);
            return read_res.error();
        }

        size_t bytes_read = read_res.unwrap();
        if (bytes_read < aligned_len)
        {
            Wingos::Space::self().release_asset(asset);
            return fc::Result<Wingos::VirtualMemoryAsset>::error("disk read returned too few bytes");
        }

        Wingos::VirtualMemoryAsset vasset = Wingos::Space::self().map_memory(asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

        return vasset;
    }

    fc::Result<void> write_small(void *buffer, uint64_t lba, uint64_t len)
    {
        if (len > MAX_IPC_BUFFER_SIZE)
        {
            return "length exceeds max ipc buffer size";
        }

        IpcMessage message = {};
        message.arguments.data[0].data = DISK_WRITE_SMALL;
        message.arguments.data[1].data = lba;
        message.arguments.data[2].data = len;
        for (size_t i = 0; i < len; i++)
        {
            message.raw_buffer[i] = ((uint8_t *)buffer)[i];
        }
        message.len = len;
        auto sended_message = connection.send_async(message);
        return {};
    }

    fc::Result<void> write(Wingos::MemoryAsset &asset, uint64_t lba, uint64_t len)
    {
        if (len < MAX_IPC_BUFFER_SIZE)
        {
            fmt::warn$("write length is smaller than max ipc buffer size, consider using write_small");
        }

        IpcMessage message = {};
        message.arguments.data[0].data = DISK_WRITE_SECTORS;
        message.arguments.data[1].data = lba;
        message.arguments.data[2].data = len;
        message.arguments.data[3].is_asset = true;
        message.arguments.data[3].asset_handle = asset.handle;
        connection.send_async(message);
        return {};
    }

    static fc::Result<DiskConnection> connect(fc::Str dev_name)
    {
        DiskConnection conn;
        auto init_conn = try$(prot::InitConnection::connect());

        auto v = try$(init_conn.get_server(dev_name, 1, 0));

        conn.connection = Wingos::Space::self().connect_by_addr(v.endpoint);

        fmt::log$("Connected to disk server at address: {} ({})", v.endpoint, conn.connection.associated_space_handle);
        return conn;
    }

    static fc::Result<DiskConnection> use_asset(uint64_t conn_handle)
    {
        DiskConnection conn;
        conn.connection = Wingos::Space::self().from_already_connected(conn_handle);

        fmt::log$("Connected to disk server at address: {} ({})", conn_handle, conn.connection.associated_space_handle);
        return conn;
    }

    static fc::Result<DiskConnection> connect_by_addr(IpcServerHandle disk_endpoint)
    {
        DiskConnection conn;
        conn.connection = Wingos::Space::self().connect_by_addr(disk_endpoint);

        fmt::log$("Connected to disk server at address: {} ({})", disk_endpoint, conn.connection.associated_space_handle);
        return conn;
    }
};
} // namespace prot
