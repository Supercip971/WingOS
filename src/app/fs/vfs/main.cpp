#include "libcore/fmt/fmt_str.hpp"
#include "libcore/str_writer.hpp"
#include "protocols/server_helper.hpp"

#include "file.hpp"
#include "fs/gpt/gpt.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "protocols/vfs/fsManager.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/ipc.h"

struct RegisteredDevicePartition
{
    size_t begin;
    size_t end;

    uint64_t id;
    IpcServerHandle endpoint;
    fc::WStr part_name;
    fc::WStr part_dev_name;
    bool has_fs;
    fc::WStr fs_name;
    IpcServerHandle fs_endpoint;
};

struct RegisteredDevice
{
    char name[80];
    IpcServerHandle endpoint;
    bool has_partitions;
    fc::Vec<RegisteredDevicePartition> partitions;
};

struct MountedDevice
{
    IpcServerHandle endpoint;
    fc::WStr path;
};

struct RegisteredFs
{
    char name[80];
    prot::DiskFsManagerConnection endpoint;
};

fc::Vec<RegisteredDevice> registered_services{};
fc::Vec<RegisteredFs> registered_fs{};
size_t mounted_devices_count = 0;

void try_create_disk_endpoint()
{
    fmt::log$("rechecking mounted filesystems...");
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

                fmt::log$("trying to mount partition {} of device {} with fs {} on {}", part.part_name.view(), part.part_dev_name.view(), fc::Str(fs.name), device.endpoint);

                auto res = fs.endpoint.mount_if_device_valid(fc::Str(device.name), device.endpoint, part.begin, part.end, part.id).unwrap();

                if (res.success)
                {
                    part.has_fs = true;
                    part.fs_endpoint = res.fs_endpoint;
                    part.fs_name = fc::WStr::copy(fc::Str(fs.name));

                    MountedDevice mdev{};
                    mdev.endpoint = res.fs_endpoint;
                    if (mounted_devices_count == 0)
                    {
                        mdev.path = fc::WStr::copy(fc::Str("/"));
                    }
                    else
                    {
                        mdev.path = fc::WStr::copy(fc::Str("/mnt/") + device.name);
                    }

                    mounted_devices_count++;

                    mount_fs(mdev.endpoint, fc::move(mdev.path)).unwrap();

                    fmt::log$("detected partition {} of device {} with fs {}", part.part_name.view(), part.part_dev_name.view(), part.fs_name.view());
                    break;
                }
            }
        }
    }
}

int main(int, char **)
{

    // attempt connection to server ID 0
    fmt::log$("started vfs");

    auto server_r = prot::ManagedServer::create_registered_server("vfs", 1, 0);
    fmt::log$("registered vfs");
    if (server_r.is_error())
    {
        fmt::err$("failed to create registered vfs server: {}", server_r.error());
        return 1;
    }

    auto server = server_r.take();

    registered_services = fc::Vec<RegisteredDevice>();
    registered_fs = fc::Vec<RegisteredFs>();

    while (true)
    {

        update_all_endpoints();
        auto received = server.try_receive();

        server.accept_connection();

        if (!received.is_error())
        {
            auto msg = fc::move(received.unwrap());

            if (msg.received.flags & IPC_MESSAGE_FLAG_DISCONNECT)
            {
                fmt::log$("Received disconnect message");

                server.disconnect(msg.connection);
                continue;
            }
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

                fmt::log$("(server) registered device: {} with endpoint: {}", device.name, device.endpoint);

                fc::Str v = fc::Str(device.name);
                auto v2_res = Wingos::parse_gpt(v);
                auto v2 = v2_res.take();

                size_t part_id = 0;
                for (auto &entry : v2.entries)
                {
                    RegisteredDevicePartition part{};
                    part.id = part_id++;
                    part.endpoint = device.endpoint;
                    fc::WStr part_name = fc::move(fmt::format_str("{}-{}", device.name, part.id).unwrap());
                    part.part_dev_name = fc::WStr::copy(part_name.view());
                    part.part_name = fc::WStr::copy(entry.name.view());
                    part.has_fs = false;
                    part.begin = entry.entry->lba_start;
                    part.end = entry.entry->lba_end;

                    fmt::log$("(server) detected partition: {} -> (LBA {} - {})", part.part_name.view(), part.part_dev_name.view(), part.begin, part.end);

                    device.partitions.push(fc::move(part));
                }

                registered_services.push(fc::move(device));
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

                fmt::log$("(server) registered filesystem: {} with endpoint: {}", fc::Str(filesystem.name), msg.received.data[1].data);

                recheck_mount = true;
                break;
            }

            case prot::VFS_ROOT_ACCESS:
            {
                fmt::log$("(server) root access requested");
                IpcMessage reply = {};
                if (mounted_devices_count > 0)
                {
                    reply.data[0].data = 1; // success

                    auto root_endpoint = VfsFileEndpoint::open_root();
                    if (root_endpoint.is_error())
                    {

                        reply.data[0].data = 0; // failure
                        reply.data[1].data = 0;
                        fmt::err$("(server) failed to open root endpoint: {}", root_endpoint.error());
                    }
                    else
                    {
                        auto root_res = root_endpoint.unwrap();
                        reply.data[1].data = root_res->server.addr();

                        fmt::log$("(server) provided root access with endpoint: {}", reply.data[1].data);
                    }
                }
                else
                {
                    fmt::err$("(server) no filesystems mounted, cannot provide root access");
                    reply.data[0].data = 0; // failure
                    reply.data[1].data = 0;
                }

                auto send_res = server.reply(fc::move(msg), reply);
                if (send_res.is_error())
                {
                    fmt::err$("(server) failed to send root access reply: {}", send_res.error());
                }

                break;
            }
            default:
            {
                fmt::log$("(server) unknown message type: {}", msg.received.data[0].data);
                break;
            }
            }

            if (recheck_mount)
            {
                try_create_disk_endpoint();
            }

            // check for all mounted filesystems updates
        }
    }
}
