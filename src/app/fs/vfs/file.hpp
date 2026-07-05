#pragma once

#include "libcore/str_writer.hpp"
#include "protocols/server_helper.hpp"

#include "libcore/result.hpp"
#include "protocols/vfs/file.hpp"
#include "protocols/vfs/fsManager.hpp"
#include "wingos-headers/ipc.h"

struct MountedFs
{
    prot::DiskFsImplementationConnection endpoint;
    fc::WStr path;
};

fc::Result<void> mount_fs(IpcServerHandle device_name, fc::WStr &&mount_path);

// app -> server -> connection to fs -> ext2
class VfsFileEndpoint
{
public:
    prot::FsFile connection_to_fs = {};

    prot::ManagedServer server = {};
    static fc::Result<VfsFileEndpoint *> open_root();
};

void update_all_endpoints();
