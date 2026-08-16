#include <stddef.h>
#include <string.h>

#include "hw/mem/addr_space.hpp"
#include "protocols/server_helper.hpp"

#include "app/timer/hpet/server.hpp"
#include "hw/acpi/rsdp.hpp"
#include "hw/acpi/rsdt.hpp"
#include "hw/hpet/hpet.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/time/time.hpp"
#include "math/range.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"
#include "wingos-headers/startup.hpp"

// source: derived from brutal OS but
// I wrote the brutal PS2 code
int main(int, char **) { return 0; };

int _main(StartupInfo *context)
{
    fmt::log$("clock from rsdp addr: {}", context->machine_context_optional._rsdp | fmt::FMT_HEX);

    uintptr_t phys_rsdp = context->machine_context_optional._rsdp - 0xffff800000000000;

    fmt::log$("preparing rsdp mapping");
    hw::acpi::prepare_mapping(phys_rsdp, [](uintptr_t addr, size_t size)
                              {
        auto vrange = math::Range<size_t>(addr, addr + size).growAlign(4096);

        fmt::log$("mapping hpet rsdp region: {} - {}", vrange.start() | fmt::FMT_HEX, vrange.end() | fmt::FMT_HEX);

        Wingos::Space::self().map_physical_memory(vrange.start(), vrange.len(), ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
        return fc::Result<size_t>{
            vrange.start()
        }; })
        .unwrap();

    fmt::log$("preparing hpet");

    hw::hpet::hpet_prepare_mapping(phys_rsdp, [](uintptr_t addr, size_t size)
                                   {
        auto vrange = math::Range<size_t>(addr, addr + size).growAlign(4096);

        fmt::log$("mapping hpet region: {} - {}", vrange.start() | fmt::FMT_HEX, vrange.end() | fmt::FMT_HEX);
        Wingos::Space::self().map_physical_memory(vrange.start(), vrange.len(), ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
        return fc::Result<size_t>{
            vrange.start()
        }; })
        .unwrap();

    hw::hpet::hpet_initialize(toVirt(phys_rsdp).as<hw::acpi::Rsdp>()).unwrap();
    // map everything

    auto server_r = (prot::ManagedServer::create_registered_server<HpetServer>("clock", 1, 0));
    auto server = std::move(server_r.unwrap());

    if (server_r.is_error())
    {
        fmt::err$("failed to create hio server: {}", server_r.error());
        return -1;
    }

    fmt::log$("started clock service");
    while (true)
    {

        bool has_waiter = false;
        auto now = hw::hpet::hpet_clock_read();

        for (auto &conn : server->connected_clients())
        {
            HPetConnection *timer = static_cast<HPetConnection *>(conn.value);
            if (timer->waiting)
            {
                if (timer->end_time <= now)
                {
                    IpcMessage reply = {};
                    reply.arg(0, 0); // success

                    timer->reply(reply, timer->waiter);
                    timer->waiting = false;
                    timer->waiter = {};
                }
                else
                {
                    has_waiter = true;
                }
            }
        }

        if (has_waiter)
        {
            server->do_receive_async();
        }
        else
        {
            server->do_receive();
        }
    }
}
