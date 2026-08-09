
#pragma once

#include "protocols/server_helper.hpp"

#include "app/fs/vfs/administration.hpp"
#include "protocols/disk/disk.hpp"
#include "protocols/vfs/fsManager.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/ipc.h"

class VfsServer : public prot::ManagedServer
{

    VfsServerCtx ctx;

public:
    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        switch (initiator.arg(0))
        {

        case prot::VFS_ACCESS_ROOT:
        case prot::VFS_ACCESS_ADMINISTRATION:
            return new VfsServerAdministration(this->ctx);
            // case prot::VFS_ACCESS_PWD:
            // unimplemented
            break;

        default:
            return "invalid open access for vfs";
        }
        return "invalid open access for vfs";
    }

    virtual fc::Result<void> after_receive() final
    {
        if (!ctx.dirty_mount)
        {
            return {};
        }

        fmt::log$("rechecking mounted filesystems...");
        for (auto &device : ctx.registered_services)
        {

            for (auto &part : device.partitions)
            {
                if (part.has_fs)
                {
                    continue;
                }

                for (auto &fs : ctx.registered_fs)
                {

                    fmt::log$("trying to mount partition {} of device {} with fs {} on {}", part.part_name.view(), part.part_dev_name.view(), fc::Str(fs.name));

                    auto res = fs.endpoint_to_fs.mount_if_device_valid(fc::Str(device.name), try$(device.connection_handle.fork_client()).raw_client().handle, part.begin, part.end, part.id).unwrap();

                    if (res.success)
                    {
                        part.has_fs = true;
                        part.fs_name = fc::WStr::copy(fc::Str(fs.name));

                        MountedDevice mdev{};
                        mdev.endpoint = res.fs_endpoint;
                        if (ctx.mounted_devices_count == 0)
                        {
                            mdev.path = fc::WStr::copy(fc::Str("/"));
                        }
                        else
                        {
                            mdev.path = fc::WStr::copy(fc::Str("/mnt/") + device.name);
                        }

                        ctx.mounted_devices_count++;

                        ctx.mount_fs(mdev.endpoint, std::move(mdev.path)).unwrap();

                        fmt::log$("detected partition {} of device {} with fs {}", part.part_name.view(), part.part_dev_name.view(), part.fs_name.view());
                        break;
                    }
                }
            }
        }
        ctx.dirty_mount = false;
        return {};
    }

    virtual ~VfsServer() {}
};
