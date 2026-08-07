#include "file.hpp"

#include "libcore/str_writer.hpp"

#include "app/fs/vfs/ctx.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "protocols/vfs/file.hpp"

MountedFs _root;

fc::Result<VfsConnectionFile *> VfsConnectionFile::open_root(VfsServerCtx &ctx)
{
    fmt::log$("VfsConnectionFile::open_root: searching for root filesystem, {} mounted", ctx.mounted_filesystems.len());
    for (size_t i = 0; i < ctx.mounted_filesystems.len(); i++)
    {
        fmt::log$("VfsFileEndpoint::open_root: checking mount {}: path={}", i, ctx.mounted_filesystems[i].path.view());
        if (ctx.mounted_filesystems[i].path.view() == (fc::Str("/")))
        {

            auto file = Wingos::Space::self().connect_by_handle(ctx.server_handle);

            auto root_endpoint = try$(ctx.mounted_filesystems[i].endpoint.create_root_endpoint());

            auto connect_res = prot::FsFile::use_connection(root_endpoint);
            if (connect_res.is_error())
            {
                fmt::err$("VFS: failed to connect to root filesystem: {}", connect_res.error());
                file.disconnect();
                return connect_res.error();
            }

            auto connect_ptr = new prot::FsFile(connect_res.take());

            ctx.openned_file.insert(file.port, connect_ptr);

            fmt::log$("VFS: connected to root fs, client handle: {}", file.port);

            VfsConnectionFile *endpoint = new VfsConnectionFile(ctx);
            endpoint->connection_to_fs = connect_ptr;
            endpoint->client_to_be_given = file;

            return endpoint;
        }
    }

    fmt::err$("VfsFileEndpoint::open_root: no root filesystem mounted");
    return fc::Result<VfsConnectionFile *>::error("no root filesystem mounted");
}
