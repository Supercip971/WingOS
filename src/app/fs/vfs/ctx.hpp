#pragma once

#include "libcore/str_writer.hpp"
#include "protocols/server_helper.hpp"

#include "iol/wingos/ipc.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/result.hpp"
#include "protocols/disk/disk.hpp"
#include "protocols/vfs/file.hpp"
#include "protocols/vfs/fsManager.hpp"

struct RegisteredDevicePartition
{
    size_t begin;
    size_t end;

    uint64_t id;
    fc::WStr part_name;
    fc::WStr part_dev_name;
    bool has_fs;
    fc::WStr fs_name;
};

struct RegisteredDevice
{
    char name[80];
    prot::DiskConnection connection_handle;
    bool has_partitions;
    fc::Vec<RegisteredDevicePartition> partitions;
};

struct MountedDevice
{
    prot::DiskFsImplementationConnection endpoint;
    fc::WStr path;
};

struct RegisteredFs
{
    char name[80];
    prot::DiskFsManagerConnection endpoint_to_fs;
};

struct MountedFs
{
    prot::DiskFsImplementationConnection endpoint;
    fc::WStr path;
};

struct VfsServerCtx
{

    prot::ManagedServer *root_vfs;
    fc::Vec<RegisteredDevice> registered_services{};
    fc::Vec<RegisteredFs> registered_fs{};

    fc::Vec<MountedFs> mounted_filesystems = {};
    size_t mounted_devices_count = 0;
    uint64_t server_handle = 0;
    bool dirty_mount = false;

    fc::Result<void> mount_fs(prot::DiskFsImplementationConnection &dev, fc::WStr &&mount_path)
    {
        for (auto &mnt : mounted_filesystems)
        {
            if (mnt.path == mount_path)
            {
                return ("mount path already in use");
            }
        }

        if (mount_path.view() == fc::Str("/"))
        {
            fmt::log$("VFS: signaled init that root fs is available");
            prot::InitConnection init_conn = prot::InitConnection::connect().unwrap();
            init_conn.signal_fs_available().unwrap();
            init_conn.end();
        }

        MountedFs mnt = {};
        mnt.endpoint = dev;
        mnt.path = std::move(mount_path);
        mounted_filesystems.push(std::move(mnt));

        return {};
    }
};
