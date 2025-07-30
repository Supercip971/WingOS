#include <stdlib.h>
#include <string.h>

#include "hw/mem/addr_space.hpp"

#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "kernel/generic/space.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libelf/elf.hpp"
#include "math/align.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"
#include "wingos-headers/syscalls.h"
const char *hello_world = "Hello, World!\n";

core::Result<size_t> execute_module(elf::ElfLoader loaded)
{

    auto subspace = Wingos::Space::self().create_space();

    auto task_asset = subspace.create_task((uintptr_t)loaded.entry_point());

    if (task_asset.handle == 0)
    {
        log::err$("failed to create task asset: {}", task_asset.handle);
        return core::Result<size_t>::error("failed to create task asset");
    }

    for (size_t i = 0; i < loaded.program_count(); i++)
    {

        auto ph = try$(loaded.program_header(i));
        if (ph.type != core::underlying_value(ElfProgramHeaderType::HEADER_LOAD))
        {
            log::warn$("skipping program header {}: type is not LOAD but {}", i, ph.type);
            continue;
        }

        log::log$("section[{}]: type: {}, flags: {}, virt_addr: {}, file_offset: {}, file_size: {}",
                  i, ph.type, ph.flags, ph.virt_addr, ph.file_offset, ph.file_size);

        auto memory = Wingos::Space::self().allocate_physical_memory(ph.mem_size, false);

        auto mapped_self = Wingos::Space::self().map_memory(memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

        void *copied_data = (void *)((uintptr_t)mapped_self.ptr());
        memset(copied_data, 0, ph.mem_size);
        memcpy(copied_data,
               (void *)((uintptr_t)loaded.range().start() + ph.file_offset),
               ph.file_size);

        log::log$("copied {} bytes from {} to {}", ph.file_size, (uintptr_t)loaded.range().start() + ph.file_offset, (uintptr_t)copied_data);

        auto moved_memory = Wingos::Space::self().move_to(subspace, memory);

        subspace.map_memory(ph.virt_addr, ph.virt_addr + ph.mem_size, moved_memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
    }

    subspace.launch_task(task_asset);

    return 0ul;
}

void start_server()
{
}

int _main(mcx::MachineContext *context)
{

    SyscallIpcCreateServer create = sys$ipc_create_server(SPACE_SELF, true);
    log::log$("created server with handle: {}", create.returned_handle);
    core::Vec<IpcConnectionHandle> connections = {};

    for (int i = 0; i < context->_modules_count; i++)
    {
        log::log$("module {}: {}", i, context->_modules[i].path);
    }

    void *ptr = malloc(1024 * 1024);

    log::log$("allocated 1MB at: {}", (uintptr_t)ptr | fmt::FMT_HEX);

    for (int i = 0; i < context->_modules_count; i++)
    {
        auto mod = context->_modules[i];

        if (core::Str(mod.path).start_with("/bin/init"))
        {
            log::log$("skipping module {}: {}", i, mod.path);
            continue;
        }
        log::log$("module {}: {}", i, mod.path);

        auto range = mod.range;
        range.start(range.start() - 0xffff800000000000);
        range.end(range.end() - 0xffff800000000000);

        auto self_mapped = Wingos::Space::self().map_physical_memory(range.start(), range.len(), ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

        auto loaded = elf::ElfLoader::load(VirtRange(
            range.start() + 0x0000002000000000,
            range.end() + 0x0000002000000000));

        (void)self_mapped;

        if (loaded.is_error())
        {
            log::err$("unable to load module {}: {}", i, loaded.error());
            continue;
        }

        log::log$("module {} loaded: {}", i, mod.path);

        execute_module(loaded.unwrap()).assert();
    }

    start_server();

    while (true)
    {
        SyscallIpcAccept accept = sys$ipc_accept(false, SPACE_SELF, create.returned_handle);
        if (accept.accepted_connection)
        {
            log::log$("accepted connection: {}", accept.connection_handle);
            connections.push(accept.connection_handle);
        }
        else
        {
        }

        for (size_t i = 0; i < connections.len(); i++)
        {
            auto connection = connections[i];

            SyscallIpcServerReceive call = sys$ipc_receive_server(false, SPACE_SELF, create.returned_handle, connection);

            if (call.contain_response)
            {
                log::log$("received message from connection {}: {}", connection, call.returned_msg_handle);
                log::log$("message: {}", call.returned_message.data[0].data);

                IpcMessage reply = {};
                reply.data[0].data = 1234;
                reply.data[0].is_asset = false;
                [[maybe_unused]] SyscallIpcReply reply_call = sys$ipc_reply(SPACE_SELF, create.returned_handle, connection, call.returned_msg_handle, reply);
            }
        }
    }
    while (true)
    {
        //   log::log$("no Hello, World!");
    }
    return 1;
}