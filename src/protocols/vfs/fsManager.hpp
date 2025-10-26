#pragma once 

#include "libcore/str.hpp"
#include "iol/wingos/ipc.hpp"
#include "protocols/init/init.hpp"

namespace prot 
{
    enum DiskFsManagerMessageType 
    {

        DISK_FS_ATTEMPT_INITIALIZE_DISK = 1, // mount a fs associated with a device, create a new endpoint for it 
        DISK_FS_UNMOUNT = 2,
    };

    struct MountedDiskResult 
    {
        IpcServerHandle fs_endpoint;
        bool success;
    };


    class DiskFsManagerConnection 
    {
        Wingos::IpcClient connection;
        
        public: 


        static core::Result<DiskFsManagerConnection> connect(core::Str fs_name)
        {
            DiskFsManagerConnection conn;
            auto reg = InitConnection::connect();
            if (reg.is_error())
            {
                return core::Result<DiskFsManagerConnection>::error("failed to connect to init");
            }
            auto v = reg.unwrap();
            auto handle = try$(v.get_server(core::Str(fs_name), 1, 0)).endpoint;
            conn.connection =  Wingos::Space::self().connect_to_ipc_server(handle);
            conn.connection.wait_for_accept();
            return core::Result<DiskFsManagerConnection>::success(core::move(conn));
        
        }

        static core::Result<DiskFsManagerConnection> connect(IpcServerHandle fs_endpoint)
        {
            DiskFsManagerConnection conn;
            conn.connection =  Wingos::Space::self().connect_to_ipc_server(fs_endpoint);
            conn.connection.wait_for_accept();
            return core::Result<DiskFsManagerConnection>::success(core::move(conn));
        
        }

        Wingos::IpcClient& raw_client() { return connection; }

        core::Result<MountedDiskResult> mount_if_device_valid(core::Str name, IpcServerHandle endpoint, size_t begin_lba, size_t end_lba, size_t part_id)
        {
            IpcMessage message = {};
            message.data[0].data = DISK_FS_ATTEMPT_INITIALIZE_DISK;
            message.data[1].data = endpoint;
            message.data[2].data = begin_lba;
            message.data[3].data = end_lba;
            message.data[4].data = part_id; // part id

            if(name.len() > 80) {
                return ("device name too long");
            }

            char name_buf[80] = {0};
            name.copy_to(name_buf, 80);
            
            for(size_t i = 0; i < name.len(); i++)
            {
                message.raw_buffer[i] = name_buf[i];
            }

            message.raw_buffer[name.len()] = 0;
            message.len = name.len();

            
            auto send_res = connection.send(message, true);
            if (send_res.is_error())
            {
                return ("failed to send validate disk message");
            }

            while(true)
            {
                auto recv_res = connection.receive_reply(send_res.unwrap());
                if (!recv_res.is_error())
                {
                    auto msg = recv_res.unwrap();
                    MountedDiskResult result {};
                    result.fs_endpoint = msg.data[0].data;
                    result.success = msg.data[0].data != 0;
                    return {result};
                }

            }
        }

    };


};
