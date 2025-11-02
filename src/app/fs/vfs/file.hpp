#pragma once 



#include "iol/wingos/ipc.hpp"
#include "libcore/str.hpp"

#include "protocols/vfs/file.hpp"
#include "protocols/server_helper.hpp"

struct MountedFs
{
    IpcServerHandle endpoint;
    core::WStr path;
};

core::Result<void> mount_fs(IpcServerHandle device_name, core::WStr&& mount_path);


// app -> server -> connection to fs -> ext2
class VfsFileEndpoint
{
    public: 

    prot::FsFile connection_to_fs = {};

    prot::ManagedServer server = {};
    static core::Result<VfsFileEndpoint*> open_root();    

};

void update_all_endpoints();