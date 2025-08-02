#include <stdlib.h>
#include <string.h>

#include "hw/mem/addr_space.hpp"

#include "arch/generic/syscalls.h"
#include "dev/pci/pci.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "json/json.hpp"
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




void start_from_pci()
{
    log::log$("Starting from PCI scan...");

    Wingos::dev::PciController pci_controller;
    pci_controller.scan_bus(0);

    for (const auto &device : pci_controller.devices)
    {
        log::log$("Found PCI Device: Bus {}, Device {}, Function {}, Vendor ID: {}, Device ID: {}, Class: {}, Subclass: {}",
                  device.bus, device.device, device.function,
                  device.vendor_id() | fmt::FMT_HEX, device.device_id() | fmt::FMT_HEX,
                  device.class_code() | fmt::FMT_HEX, device.subclass() | fmt::FMT_HEX);
    }
}
int _main(mcx::MachineContext *context)
{

    auto server = Wingos::Space::self().create_ipc_server(true);

    log::log$("created server with handle: {}", server.handle);
   
    for (int i = 0; i < context->_modules_count; i++)
    {
        log::log$("module {}: {}", i, context->_modules[i].path);
    }



    mcx::MachineContextModule config_module = {};
    for(int i = 0; i < context->_modules_count; i++)
    {
        auto mod = context->_modules[i];

        if (core::Str(mod.path) == core::Str("/config/init-services.json"))
        {
            log::log$("found config module: {}", mod.path);
            config_module = mod;
            break;
        }
    }

    if (config_module.path[0] == '\0')
    {
        log::err$("no config module found, cannot continue");
        return 1;
    }

    auto config_range = config_module.range;
    config_range.start(config_range.start() - 0xffff800000000000);
    config_range.end(config_range.end() - 0xffff800000000000);

    Wingos::Space::self().map_physical_memory(config_range.start(), config_range.len(), ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
    

    auto loaded_config = (void*)(config_range.start() + 0x0000002000000000);
    
    core::Vec<core::Str> module_service;
    core::Vec<core::Str> disk_service;

    auto dat = core::Str((const char*)loaded_config, config_range.len());
    
    auto jsond = (wjson::Json::parse(dat));
    if (jsond.is_error())
    {
        log::err$("failed to parse config: {}", jsond.error());
        return 1;
    }

    auto json = jsond.unwrap();

    auto modules = (json.root().get("modules").unwrap())->as_array().unwrap();



    for ( auto &l : *modules)
    {
        auto name = l["name"]->as_string().unwrap();
        auto path = l["path"]->as_string().unwrap();

        log::log$("module: {}, path: {}", name, path);
        module_service.push(core::Str(path));
    }

    for (int i = 0; i < context->_modules_count; i++)
    {
        auto mod = context->_modules[i];

        if(!module_service.contain(core::Str(mod.path)))
        {
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
    
    start_from_pci();


    while (true)
    {

        auto conn = server.accept();
        if(!conn.is_error())
        {
            log::log$("(server) accepted connection: {}", conn.unwrap()->handle);
        }

        auto received = server.receive();

        if(!received.is_error())
        {
            auto msg = received.unwrap();
            log::log$("(server) received message: {}", msg.received.data[0].data);

            IpcMessage reply = {};
            reply.data[0].data = 1234;
            reply.data[0].is_asset = false;

            server.reply(core::move(msg), reply).assert();
        }

    }
    while (true)
    {
        //   log::log$("no Hello, World!");
    }
    return 1;
}