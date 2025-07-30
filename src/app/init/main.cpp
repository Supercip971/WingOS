#include <stdlib.h>

#include "arch/generic/syscalls.h"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libelf/elf.hpp"
#include "math/align.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"
#include "wingos-headers/syscalls.h"

#include <string.h>
#include "hw/mem/addr_space.hpp"
const char *hello_world = "Hello, World!\n";



core::Result<size_t> execute_module(elf::ElfLoader loaded)
{
  
    auto space_res = sys$space_create(0, 0, 0);
    
    auto task_asset = sys$task_create(space_res.returned_handle, (uintptr_t)loaded.entry_point(), 0, 0, 0, 0);

    if (task_asset.returned_handle == 0)
    {
        log::err$("failed to create task asset: {}", task_asset.returned_handle);
        return core::Result<size_t>::error("failed to create task asset");
    }

    
    for(size_t i = 0; i < loaded.program_count(); i++)
    {
 
        auto ph = try$(loaded.program_header(i));
        if (ph.type != core::underlying_value(ElfProgramHeaderType::HEADER_LOAD))
        {
            log::warn$("skipping program header {}: type is not LOAD but {}", i, ph.type);
            continue;
        }

        log::log$("section[{}]: type: {}, flags: {}, virt_addr: {}, file_offset: {}, file_size: {}",
                  i, ph.type, ph.flags, ph.virt_addr, ph.file_offset, ph.file_size);

        size_t page_count = math::alignUp(ph.mem_size, 4096ul) / 4096;
        size_t mem_size = page_count * 4096;
        if (page_count == 0)
        {
            log::warn$("skipping program header {}: page count is 0", i);
            continue;
        }
        auto mem_asset_res = sys$mem_own(0, mem_size, 0);

        auto self_addr = 0x0000002000000000 + mem_asset_res.addr;
       
        auto self_mapped = sys$map_create(0, self_addr,
                           math::alignUp(self_addr + mem_size, 4096ul),
                           mem_asset_res.returned_handle, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
        

        
        void * copied_data = (void *)((uintptr_t)self_addr );
        memset(copied_data, 0, ph.mem_size);
        memcpy(copied_data,
               (void *)((uintptr_t)loaded.range().start() + ph.file_offset),
               ph.file_size);

        log::log$("copied {} bytes from {} to {}", ph.file_size, (uintptr_t)loaded.range().start() + ph.file_offset, (uintptr_t)copied_data);
        


        auto new_handle = sys$asset_move(0, space_res.returned_handle, mem_asset_res.returned_handle);
  
        auto target_mapped = sys$map_create(space_res.returned_handle, ph.virt_addr,
                           math::alignUp(ph.virt_addr + mem_size, 4096ul),
                           new_handle.returned_handle_in_space, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
        (void)target_mapped;
        (void)self_mapped;
    }

    sys$task_launch(space_res.returned_handle, task_asset.returned_handle, 0, 0, 0, 0);
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
    
    for(int i = 0; i < context->_modules_count; i++)
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
        auto range_aligned = range.growAlign(4096);
        
        


        auto phys_mem = sys$mem_own(SPACE_SELF, range_aligned.len() , range_aligned.start());
        auto self_mapped = sys$map_create(SPACE_SELF, 0x0000002000000000 + phys_mem.addr,
                           math::alignUp(0x0000002000000000 +range_aligned.len() + phys_mem.addr, 4096ul),
                           phys_mem.returned_handle,
                            ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
 

        auto loaded = elf::ElfLoader::load(VirtRange(
            range.start() + 0x0000002000000000,
            range.end() + 0x0000002000000000
        ));

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
    
   while(true)
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

        for(size_t i = 0; i < connections.len(); i++)
        {
            auto connection = connections[i];
            
            SyscallIpcServerReceive call = sys$ipc_receive_server(false, SPACE_SELF, create.returned_handle, connection);
            
            if(call.contain_response)
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