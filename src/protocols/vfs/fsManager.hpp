#pragma once

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/ipc.h"

namespace prot
{

class DiskFsImplementationConnection
{
    Wingos::IpcClient connection;

public:
    static fc::Result<DiskFsImplementationConnection> connect(IpcServerHandle fs_endpoint)
    {
        DiskFsImplementationConnection conn;
        conn.connection = Wingos::Space::self().connect_by_addr(fs_endpoint);
        return fc::Result<DiskFsImplementationConnection>::success(std::move(conn));
    }

    static fc::Result<DiskFsImplementationConnection> use(IpcConnectionHandle conn)
    {
        DiskFsImplementationConnection imp;
        imp.connection = Wingos::Space::self().from_already_connected(conn);
        return imp;
    }

    fc::Result<IpcServerHandle> create_root_endpoint()
    {
        IpcMessage message = {};
        message.arguments.data[0].data = VFS_ACCESS_ROOT;

        try$(connection.call(message));

        IpcServerHandle endpoint = message.asset(1);
        return endpoint;
    };
};

struct MountedDiskResult
{
    prot::DiskFsImplementationConnection fs_endpoint;
    bool success;
};

class DiskFsManagerConnection
{
    Wingos::IpcClient connection;

public:
    static fc::Result<DiskFsManagerConnection> connect(fc::Str fs_name)
    {
        DiskFsManagerConnection conn = {};
        auto reg = InitConnection::connect();
        if (reg.is_error())
        {
            return fc::Result<DiskFsManagerConnection>::error("failed to connect to init");
        }
        auto v = reg.unwrap();
        auto handle = try$(v.get_server(fc::Str(fs_name), 1, 0)).endpoint;
        conn.connection = Wingos::Space::self().connect_by_addr(handle);
        return fc::Result<DiskFsManagerConnection>::success(std::move(conn));
    }

    static fc::Result<DiskFsManagerConnection> connect_by_addr(IpcServerHandle fs_endpoint)
    {
        DiskFsManagerConnection conn = {};
        conn.connection = Wingos::Space::self().connect_by_addr(fs_endpoint);
        return fc::Result<DiskFsManagerConnection>::success(std::move(conn));
    }

    Wingos::IpcClient &raw_client() { return connection; }

    fc::Result<MountedDiskResult> mount_if_device_valid(fc::Str name, IpcServerHandle endpoint, size_t begin_lba, size_t end_lba, size_t part_id)
    {
        IpcMessage message = {};
        message.arguments.data[0].data = VFS_DISK_ATTEMPT_INITIALIZE;
        message.arguments.data[1].data = endpoint;
        message.arguments.data[2].data = begin_lba;
        message.arguments.data[3].data = end_lba;
        message.arguments.data[4].data = part_id; // part id

        if (name.len() > 80)
        {
            return ("device name too long");
        }

        char name_buf[80] = {0};
        name.copy_to(name_buf, 80);

        for (size_t i = 0; i < name.len(); i++)
        {
            message.raw_buffer[i] = name_buf[i];
        }

        message.raw_buffer[name.len()] = 0;
        message.len = name.len();

        try$(connection.call(message));

        MountedDiskResult result{};
        result.fs_endpoint = DiskFsImplementationConnection::use(message.asset(1)).take();
        result.success = message.arguments.data[0].data != 0;
        return result;
    };
};
} // namespace prot
