#include <string.h>

#include "module_startup.hpp"

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

VirtRange map_mcx_address(mcx::MemoryRange range)
{
    range.start(range.start() - 0xffff800000000000);
    range.end(range.end() - 0xffff800000000000);

    Wingos::Space::self().map_physical_memory(range.start(), range.len(), ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

    void *mem = (void *)(range.start() + 0x0000002000000000);

    return VirtRange((uintptr_t)mem, (uintptr_t)mem + range.len());
}
core::Result<size_t> start_service(mcx::MachineContextModule mod)
{
    auto vrange = map_mcx_address(mod.range);
    auto loaded = elf::ElfLoader::load(vrange);

    if (loaded.is_error())
    {
        log::err$("[INIT] unable to load module {}: {}", loaded.error());
        return loaded.error();
    }

    auto v = execute_module(loaded.unwrap());

    log::log$("[INIT] started boot module: {}", mod.path);
    return v;
}

core::Result<size_t> start_service_fs(core::Str const &path)
{

    log::log$("[INIT] starting module from fs: {}", path);
    log::warn$("[INIT] starting module from fs is not implemented yet");

    return core::Result<size_t>::error("not implemented");
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

    log::warn$("[INIT] no module found with path: {}", path);

    return start_service_fs(path);
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

    auto config_range = map_mcx_address(config_module.range);
    auto loaded_config = (void *)(config_range.start());

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

core::Result<bool> try_startup_modules_cycle_one(mcx::MachineContext *context)
{
    for (size_t i = 0; i < module_to_launch.len(); i++)
    {
        auto &mod = module_to_launch[i];

        bool can_start = true;
        for (size_t d = 0; d < mod.deps.len(); d++)
        {
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
        try$(start_service(context, mod.name));

        started_modules.push(mod.name);
        module_to_launch.pop(i);

        return true;
    }

    return false;
}
core::Result<void> try_startup_modules_cycle(mcx::MachineContext *context)
{

    while (try$(try_startup_modules_cycle_one(context)))
    {
    };

    return {};
}

mcx::MachineContext *gmcx = nullptr;

core::Result<void> startup_module(mcx::MachineContext *context)
{
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