#include "file.hpp"

#include "libcore/str_writer.hpp"

#include "app/fs/vfs/ctx.hpp"
#include "iol/wingos/ipc.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "protocols/vfs/file.hpp"

MountedFs _root;

fc::Result<Wingos::IpcClient> VfsConnectionFile::open_root(VfsServerCtx &ctx)
{
    for (size_t i = 0; i < ctx.mounted_filesystems.len(); i++)
    {
        if (ctx.mounted_filesystems[i].path.view() == (fc::Str("/")))
        {

            auto root_endpoint = try$(ctx.mounted_filesystems[i].endpoint.create_root_endpoint());

            auto connect_res = prot::FsFile::use_connection(root_endpoint);
            if (connect_res.is_error())
            {
                fmt::err$("VFS: failed to connect to root filesystem: {}", connect_res.error());
                return connect_res.error();
            }

            auto connect_ptr = new prot::FsFile(connect_res.take());

            VfsConnectionFile *endpoint = new VfsConnectionFile(ctx);
            endpoint->connection_to_fs = connect_ptr;

            return ctx.root_vfs->create_connection(endpoint);
        }
    }

    return ("no root filesystem mounted");
}
