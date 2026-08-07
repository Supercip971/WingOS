#pragma once

#include "iol/wingos/ipc.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/file.hpp"
#include "wingos-headers/ipc.h"

namespace prot
{

enum VfsConnectionMessageType
{
    VFS_ACCESS_ADMINISTRATION = 0,
    VFS_ACCESS_PWD = 2,
    VFS_ACCESS_ROOT = 3,
};

enum VfsAdminMessageType
{
    VFS_REGISTER_DISK = 1000'0,
    VFS_MOUNT = 1000'1,
    VFS_UNMOUNT = 1000'2,
    VFS_REGISTER_FS = 1000'3,
    // mainly for disk server
    VFS_DISK_ATTEMPT_INITIALIZE = 2000'1,
    VFS_DISK_UNMOUNT = 2000'2
};

struct VfsRegister
{
    fc::Str device_name;
    IpcServerHandle device_endpoint;
};

struct VfsRegisterFs
{
    fc::Str fs_name;
    IpcServerHandle fs_endpoint;
};

struct VfsMount
{
    fc::Str path;
    fc::Str device_name;
};

class VfsConnection : public fc::NoCopy
{
    Wingos::IpcClient connection;
    bool connected = false;

public:
    Wingos::IpcClient &raw_client() { return connection; }

    ~VfsConnection()
    {
        if (connected)
        {
            connection.disconnect();
            connected = false;
        }
    }

    // Disable copy to prevent double-disconnect
    VfsConnection(const VfsConnection &) = delete;
    VfsConnection &operator=(const VfsConnection &) = delete;

    // Enable move
    VfsConnection(VfsConnection &&other) : connection(other.connection), connected(other.connected)
    {

        other.connected = false;
    }

    VfsConnection &operator=(VfsConnection &&other)
    {
        if (this != &other)
        {
            if (connected)
            {
                connection.disconnect();
            }
            connection = other.connection;
            connected = other.connected;
            other.connected = false;
        }
        return *this;
    }

    VfsConnection() = default;

    fc::Result<void> register_device(fc::Str name, IpcServerHandle endpoint)
    {
        IpcMessage message = {};
        message.arguments.data[0].data = VFS_REGISTER_DISK;
        message.arguments.data[1].data = endpoint;

        if (name.len() > 80)
        {
            return ("device name too long");
        }

        for (size_t i = 0; i < name.len(); i++)
        {
            message.raw_buffer[i] = name[i];
        }
        message.raw_buffer[name.len()] = 0;
        message.len = name.len();
        connection.send(message);
        return {};
    }

    static fc::Result<VfsConnection> connect(IpcServerHandle handle)
    {
        VfsConnection vfs_conn;
        vfs_conn.connection = Wingos::Space::self().connect_by_addr(handle);
        return vfs_conn;
    }

    static fc::Result<VfsConnection> connect()
    {
        VfsConnection vfs_conn = {};
        auto reg = InitConnection::connect();
        if (reg.is_error())
        {
            return fc::Result<VfsConnection>::error("failed to connect to init");
        }
        auto v = reg.unwrap();
        auto handle = try$(v.get_server(fc::Str("vfs"), 1, 0)).endpoint;

        vfs_conn.connection = Wingos::Space::self().connect_by_addr(handle);
        return vfs_conn;
    }

    fc::Result<void> register_fs(fc::Str name, IpcServerHandle endpoint)
    {
        IpcMessage message = {};
        message.arguments.data[0].data = VFS_REGISTER_FS;
        message.arguments.data[1].data = endpoint;

        if (name.len() > 80)
        {
            return ("fs name too long");
        }

        for (size_t i = 0; i < name.len(); i++)
        {
            message.raw_buffer[i] = name[i];
        }
        message.raw_buffer[name.len()] = 0;
        message.len = name.len();
        auto sended_message = connection.send(message);
        return {};
    }

    fc::Result<FsFile> open_root()
    {
        IpcMessage message = {};
        message.arguments.data[0].data = VFS_ACCESS_ROOT;
        try$(connection.call(message));

        if (message.arguments.data[0].data == 0)
        {
            return ("failed to obtain root access");
        }
        IpcServerHandle file_endpoint = message.arguments.data[1].data;
        auto file_res = FsFile::connect_by_handle(file_endpoint);

        return file_res;
    }

    fc::Result<FsFile> open_path(fc::Str const &path)
    {
        fmt::log$("VfsConnection::open_path: opening path {}", path.view());
        if (path[0] != '/')
        {
            fmt::warn$("FIXME: path must be absolute");
            return ("only absolute paths are supported");
        }

        fmt::log$("opening path {}", path.view());
        auto root_res = try$(open_root());
        auto current_dir = std::move(root_res);
        auto components = path.substr(1).split('/');

        for (size_t i = 0; i < components.len(); i++)
        {
            fmt::log$("path component {}: {}", i, components[i].view());
        }
        for (size_t i = 0; i < components.len(); i++)
        {
            auto next_dir_res = try$(current_dir.open_file(components[i]));
            current_dir.close();
            current_dir = std::move(next_dir_res);
        }

        return current_dir;
    }
};

}; // namespace prot
