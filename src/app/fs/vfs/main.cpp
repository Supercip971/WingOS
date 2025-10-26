#include "arch/generic/syscalls.h"
#include "fs/gpt/gpt.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/init/init.hpp"
#include "libcore/fmt/fmt_str.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/syscalls.h"
#include "protocols/vfs/fsManager.hpp"
#include "protocols/server_helper.hpp"
struct RegisteredDevicePartition
{
    size_t begin;
    size_t end;


    uint64_t id;
    IpcServerHandle endpoint;
    core::WStr part_name;
    core::WStr part_dev_name;
    bool has_fs;
    core::WStr fs_name;
    IpcServerHandle fs_endpoint;
};

struct RegisteredDevice
{
    char name[80];
    IpcServerHandle endpoint;
    bool has_partitions;
    core::Vec<RegisteredDevicePartition> partitions;
};

struct RegisteredFs
{
    char name[80];
    prot::DiskFsManagerConnection endpoint;
};
core::Vec<RegisteredDevice> registered_services;
core::Vec<RegisteredFs> registered_fs;

void try_create_disk_endpoint()
{
    log::log$("rechecking mounted filesystems...");
    for (auto &device : registered_services)
    {

        for (auto &part : device.partitions)
        {
            if (part.has_fs)
            {
                continue;
            }

            for (auto &fs : registered_fs)
            {

                log::log$("trying to mount partition {} of device {} with fs {} on {}", part.part_name.view(), part.part_dev_name.view(), core::Str(fs.name), device.endpoint);

                auto res = fs.endpoint.mount_if_device_valid(core::Str(device.name), device.endpoint, part.begin, part.end, part.id).unwrap();

                if(res.success)
                {
                    part.has_fs = true;
                    part.fs_endpoint = res.fs_endpoint;
                    part.fs_name = core::WStr::copy(core::Str(fs.name));
                    log::log$("detected partition {} of device {} with fs {}", part.part_name.view(), part.part_dev_name.view(), part.fs_name.view());
                    break;
                }
            }
        }
    }
}
int _main(mcx::MachineContext *)
{

    // attempt connection to server ID 0


    auto server = prot::ManagedServer::create_registered_server("vfs").unwrap();
    
    registered_services = core::Vec<RegisteredDevice>();
    registered_fs = core::Vec<RegisteredFs>();

    while (true)
    {
        server.accept_connection();
       

        auto received = server.try_receive();

        if (!received.is_error())
        {
            auto msg = received.unwrap();

            bool recheck_mount = false;
            switch (msg.received.data[0].data)
            {
            case prot::VFS_REGISTER:
            {
                
                RegisteredDevice device{};
                for (size_t i = 0; i < 80 && msg.received.raw_buffer[i] != 0; i++)
                {
                    device.name[i] = msg.received.raw_buffer[i];
                }
                device.endpoint = msg.received.data[1].data;
                device.has_partitions = false;

                recheck_mount = true;

                log::log$("(server) registered device: {} with endpoint: {}", device.name, device.endpoint);

                core::Str v = core::Str(device.name);
                auto v2 = Wingos::parse_gpt(v).unwrap();


                size_t part_id = 0;
                for(auto& entry : v2.entries)
                {
                    RegisteredDevicePartition part{};
                    part.id = part_id++;   
                    part.endpoint = device.endpoint;
                    core::WStr part_name = fmt::format_str("{}-{}", device.name, part.id).unwrap();
                    part.part_dev_name = core::WStr::copy(part_name.view());
                    part.part_name = core::WStr::copy(entry.name.view());
                    part.has_fs = false;
                    part.begin = entry.entry->lba_start;
                    part.end = entry.entry->lba_end;
                    


                    log::log$("(server) detected partition: {} -> (LBA {} - {})", part.part_name.view(), part.part_dev_name.view(),  part.begin, part.end);

                    device.partitions.push(core::move(part));
                }

                registered_services.push(core::move(device));
                (void)v2;

                recheck_mount = true;
                break;
            }
            case prot::VFS_REGISTER_FS:
            {
                RegisteredFs filesystem{};
                for (size_t i = 0; i < msg.received.len; i++)
                {
                    filesystem.name[i] = msg.received.raw_buffer[i];
                }
                filesystem.endpoint = prot::DiskFsManagerConnection::connect(msg.received.data[1].data).unwrap();

                registered_fs.push(filesystem);

                log::log$("(server) registered filesystem: {} with endpoint: {}", core::Str(filesystem.name), msg.received.data[1].data);

                recheck_mount = true;
                break;
            }
            default:
            {
                log::log$("(server) unknown message type: {}", msg.received.data[0].data);
                break;
            }
            }

            if (recheck_mount)
            {
                try_create_disk_endpoint();
            }
        }
    }
}