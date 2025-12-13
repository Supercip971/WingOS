#pragma once

#include "iol/wingos/ipc.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/file.hpp"
#include "wingos-headers/ipc.h"
namespace prot
{
enum VfsMessageType
{
    VFS_REGISTER = 0,
    VFS_MOUNT = 1,
    VFS_UNMOUNT = 2,
    VFS_ROOT_ACCESS = 3,
    VFS_PWD_ACCESS = 4,
    VFS_REGISTER_FS = 13,
};

struct VfsRegister
{
    core::Str device_name;
    IpcServerHandle device_endpoint;
};

struct VfsRegisterFs
{
    core::Str fs_name;
    IpcServerHandle fs_endpoint;
};

struct VfsMount
{
    core::Str path;
    core::Str device_name;
};

class VfsConnection : public core::NoCopy
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
    VfsConnection(const VfsConnection&) = delete;
    VfsConnection& operator=(const VfsConnection&) = delete;

    // Enable move
    VfsConnection(VfsConnection&& other) : connection(other.connection), connected(other.connected)
    {

        other.connected = false;
    }
    VfsConnection& operator=(VfsConnection&& other)
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

    core::Result<void> register_device(core::Str name, IpcServerHandle endpoint)
    {
        IpcMessage message = {};
        message.data[0].data = VFS_REGISTER;
        message.data[1].data = endpoint;

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
        auto sended_message = connection.send(message, false);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return "failed to send register device message";
        }
        (void)message_handle;

        return {};
    }
    static core::Result<VfsConnection> connect(IpcServerHandle handle)
    {
        VfsConnection vfs_conn;
        vfs_conn.connection = Wingos::Space::self().connect_to_ipc_server(handle);
        vfs_conn.connection.wait_for_accept();
        vfs_conn.connected = true;
        return vfs_conn;
    }


    static core::Result<VfsConnection> connect()
    {
        VfsConnection vfs_conn;
        auto reg = InitConnection::connect();
        if (reg.is_error())
        {
            return core::Result<VfsConnection>::error("failed to connect to init");
        }
        auto v = reg.unwrap();
        auto handle = try$(v.get_server(core::Str("vfs"), 1, 0)).endpoint;
        vfs_conn.connection = Wingos::Space::self().connect_to_ipc_server(handle);
        vfs_conn.connection.wait_for_accept();
        vfs_conn.connected = true;
        return vfs_conn;
    }




    core::Result<void> register_fs(core::Str name, IpcServerHandle endpoint)
    {
        IpcMessage message = {};
        message.data[0].data = VFS_REGISTER_FS;
        message.data[1].data = endpoint;

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
        auto sended_message = connection.send(message, false);

        if (sended_message.is_error())
        {
            return "failed to send register fs message";
        }

        return {};
    }

    core::Result<FsFile> open_root()
    {
        IpcMessage message = {};
        message.data[0].data = VFS_ROOT_ACCESS;
        auto msg = try$(connection.call(message));

        if (msg.data[0].data == 0)
        {
            return ("failed to obtain root access");
        }
        IpcServerHandle file_endpoint = msg.data[1].data;
        auto file_res = FsFile::connect(file_endpoint);

        return file_res;
    }


    core::Result<FsFile> open_path(core::Str const & path)
    {
        log::log$("VfsConnection::open_path: opening path {}", path.view());
        if(path[0] != '/')
        {
            log::warn$("FIXME: path must be absolute");
            return ("only absolute paths are supported");
        }

        log::log$("opening path {}", path.view());
        auto root_res = try$(open_root());
        auto current_dir = core::move(root_res);
        auto components = path.substr(1).split('/');

        for(size_t i = 0; i < components.len(); i++)
        {
            log::log$("path component {}: {}", i, components[i].view());
        }
        for(size_t i = 0; i < components.len(); i++)
        {
            auto next_dir_res = try$(current_dir.open_file(components[i]));
            current_dir.close();
            current_dir = core::move(next_dir_res);
        }

        return current_dir;
    }
};

}; // namespace prot
