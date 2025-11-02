#pragma once 

#include "libcore/str.hpp"
#include "iol/wingos/ipc.hpp"
#include "protocols/init/init.hpp"

#include "protocols/vfs/file.hpp"
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

    class VfsConnection 
    {
        Wingos::IpcClient connection;
        
        public: 

        Wingos::IpcClient& raw_client() { return connection; }

        core::Result<void> register_device(core::Str name, IpcServerHandle endpoint)
        {
            IpcMessage message = {};
            message.data[0].data = VFS_REGISTER;
            message.data[1].data = endpoint;

            if(name.len() > 80) {
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
            vfs_conn.connection =  Wingos::Space::self().connect_to_ipc_server(handle);
            vfs_conn.connection.wait_for_accept();
            return core::Result<VfsConnection>::success(core::move(vfs_conn));
        }


        core::Result<void> register_fs(core::Str name, IpcServerHandle endpoint)
        {
            IpcMessage message = {};
            message.data[0].data = VFS_REGISTER_FS;
            message.data[1].data = endpoint;

            if(name.len() > 80) {
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
            auto sended_message = connection.send(message, true);
            auto message_handle = sended_message.unwrap();
            if (sended_message.is_error())
            {
                return ("failed to send open root message");
            }

            while(true)
            {
                auto received = connection.receive_reply(message_handle);
                if (!received.is_error())
                {
                    auto msg = received.take();

                    if(msg.data[0].data == 0)
                    {
                        return ("failed to obtain root access");
                    }
                    IpcServerHandle file_endpoint = msg.data[1].data;
                    auto file_res = FsFile::connect(file_endpoint);
                    if(file_res.is_error())
                    {
                        return file_res.error();
                    }
                    return file_res.unwrap();

                }
            }
        }
    };


};
