#include "module_startup.hpp"
#include <string.h>

#include "dev/pci/classes.hpp"
#include "dev/pci/pci.hpp"
#include "iol/wingos/space.hpp"
#include "json/json.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libelf/elf.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/asset.h"

core::Vec<core::Str> module_service = {};
core::Vec<core::Str> disk_service = {};

struct ModuleLaunch
{
    core::Str name;
    core::Vec<core::Str> deps;
};
core::Vec<ModuleLaunch> module_to_launch = {};
core::Vec<core::Str> started_modules = {};

core::Vec<core::Str> started_services = {};
wjson::Json json = {};

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

        //  log::log$("section[{}]: type: {}, flags: {}, virt_addr: {}, file_offset: {}, file_size: {}, mem_size: {}",
        //            i, ph.type | fmt::FMT_HEX, ph.flags | fmt::FMT_HEX, ph.virt_addr | fmt::FMT_HEX, ph.file_offset | fmt::FMT_HEX, ph.file_size | fmt::FMT_HEX,
        //          ph.mem_size | fmt::FMT_HEX);
        //
        auto memory = Wingos::Space::self().allocate_physical_memory(ph.mem_size, false);

        auto mapped_self = Wingos::Space::self().map_memory(memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

        void *copied_data = (void *)((uintptr_t)mapped_self.ptr());
        memset(copied_data, 0, ph.mem_size);
        memcpy(copied_data,
               (void *)((uintptr_t)loaded.range().start() + ph.file_offset),
               ph.file_size);

        //   log::log$("copied {} bytes from {} to {}", ph.file_size | fmt::FMT_HEX, (uintptr_t)loaded.range().start() + ph.file_offset | fmt::FMT_HEX, (uintptr_t)copied_data | fmt::FMT_HEX);

        auto moved_memory = Wingos::Space::self().move_to(subspace, memory);

        subspace.map_memory(ph.virt_addr, ph.virt_addr + ph.mem_size, moved_memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
    }

    subspace.launch_task(task_asset);

    log::log$("[INIT] launched module");

    return 0ul;
}

core::Result<size_t> start_service(mcx::MachineContextModule mod)
{

    log::log$("[INIT] starting module: {}", mod.path);

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
        log::err$("[INIT] unable to load module {}: {}", loaded.error());
        return loaded.error();
    }

    auto v = execute_module(loaded.unwrap());

    return v;
}
core::Result<size_t> start_service(mcx::MachineContext *context, core::Str path)
{
    for (int i = 0; i < context->_modules_count; i++)
    {
        auto mod = context->_modules[i];

        if (core::Str(mod.path) == path)
        {
            return start_service(mod);
        }
    }

    log::err$("[INIT] no module found with path: {}", path);
    return core::Result<size_t>::error("module not found");
}
void start_from_pci(wjson::JsonValue *pjson)
{
    log::log$("Starting from PCI scan...");

    Wingos::dev::PciController pci_controller;
    pci_controller.scan_bus(0);

    auto drivers_json = (*pjson)[("drivers")]->as_array().unwrap();
    for (const auto &device : pci_controller.devices)
    {
        log::log$("Found PCI Device: Bus {}, Device {}, Function {}, Vendor ID: {}, Device ID: {}, Class: {}, Subclass: {}",
                  device.bus, device.device, device.function,
                  device.vendor_id() | fmt::FMT_HEX, device.device_id() | fmt::FMT_HEX,
                  device.class_code() | fmt::FMT_HEX, device.subclass() | fmt::FMT_HEX);

        Wingos::dev::log_dev(device.class_code(), device.subclass());
        for (auto &driver : *drivers_json)
        {
            auto pci_req = driver.get("pci");
            if (pci_req.is_error())
            {
                continue;
            }

            auto pci_req_obj = pci_req.unwrap();

            if ((*pci_req_obj)["class-code"]->as_number().unwrap() == device.class_code() &&
                (*pci_req_obj)["subclass-code"]->as_number().unwrap() == device.subclass())
            {
                log::log$("Found matching driver for device: {}", driver["name"]->as_string().unwrap());

                auto path = driver["path"]->as_string().unwrap();
                log::log$("Loading driver from path: {}", path);

                ModuleLaunch ml;
                ml.name = path;
                auto deps_json = driver.get("requires");
                if (!deps_json.is_error())
                {
                    auto deps_array = deps_json.unwrap()->as_array().unwrap();
                    for (auto &d : *deps_array)
                    {
                        log::log$("Driver dependency: {}", d.as_string().unwrap());
                        ml.deps.push(d.as_string().unwrap());
                    }
                }
                module_to_launch.push(ml);
                //                start_service(context, path);
                //                log::log$("Driver started: {}", path);
            }
        }
    }
}

core::Result<void> load_module_config(mcx::MachineContext *context)
{
    module_service = {};
    disk_service = {};
    json = {};
    for (int i = 0; i < context->_modules_count; i++)
    {
        log::log$("module {}: {}", i, context->_modules[i].path);
    }

    mcx::MachineContextModule config_module = {};
    for (int i = 0; i < context->_modules_count; i++)
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
        return "no config module found, cannot continue";
    }

    auto config_range = config_module.range;
    config_range.start(config_range.start() - 0xffff800000000000);
    config_range.end(config_range.end() - 0xffff800000000000);

    Wingos::Space::self().map_physical_memory(config_range.start(), config_range.len(), ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

    auto loaded_config = (void *)(config_range.start() + 0x0000002000000000);

    auto dat = core::Str((const char *)loaded_config, config_range.len());

    auto jsond = (wjson::Json::parse(dat));
    if (jsond.is_error())
    {
        log::err$("failed to parse config: {}", jsond.error());
        return jsond.error();
    }

    json = jsond.unwrap();

    auto modules = (json.root().get("modules").unwrap())->as_array().unwrap();

    for (auto &l : *modules)
    {
        auto name = l["name"]->as_string().unwrap();
        auto path = l["path"]->as_string().unwrap();

        log::log$("module: {}, path: {}", name, path);
        ModuleLaunch ml;
        ml.name = path;
        auto deps_json = l.get("requires");
        if (!deps_json.is_error())
        {
            auto deps_array = deps_json.unwrap()->as_array().unwrap();
            for (auto &d : *deps_array)
            {

                ml.deps.push(d.as_string().unwrap());
            }
        }
        module_to_launch.push(ml);
        // module_service.push(core::Str(path));
    }

    return {};
}

core::Result<void> try_startup_modules_cycle(mcx::MachineContext *context)
{
    bool started_one = true;

    while (started_one)
    {
        started_one = false;
        for (size_t i = 0; i < module_to_launch.len(); i++)
        {
            auto &mod = module_to_launch[i];

            bool can_start = true;
            for (size_t d = 0; d < mod.deps.len(); d++)
            {
                log::log$("module {} depends on {}, checking... against: {}", mod.name, mod.deps[d]);
                if (!started_services.contain(mod.deps[d]))
                {

                    can_start = false;
                    break;
                }
            }

            if (!can_start)
            {
                continue;
            }
            log::log$("checking module {} for startup, can_start: {}", mod.name, can_start);

            auto res = start_service(context, mod.name);
            if (res.is_error())
            {
                log::err$("failed to start module {}: {}", mod.name, res.error());
                return res.error();
            }
            started_modules.push(mod.name);
            started_one = true;
            module_to_launch.pop(i);
            break;
        }
    }

    return {};
}

mcx::MachineContext *gmcx = nullptr;

core::Result<void> startup_module(mcx::MachineContext *context)
{
    log::log$("starting modules...");

    gmcx = context;

    load_module_config(context);
    auto root = json.root();

    start_from_pci(&root);

    auto res = try_startup_modules_cycle(context);
    if (res.is_error())
    {
        log::err$("failed to startup modules cycle: {}", res.error());
    }

    return {};
}

core::Result<void> service_startup_callback(core::Str service_name)
{

    if (gmcx == nullptr)
    {
        return "machine context is null";
    }
    started_services.push(service_name);
    auto res = try_startup_modules_cycle(gmcx);

    return {};
}