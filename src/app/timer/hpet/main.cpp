#include <stddef.h>
#include <string.h>

#include "hw/mem/addr_space.hpp"
#include "protocols/server_helper.hpp"

#include "hw/acpi/rsdp.hpp"
#include "hw/acpi/rsdt.hpp"
#include "hw/hpet/hpet.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/time/time.hpp"
#include "libcore/type-utils.hpp"
#include "math/range.hpp"
#include "protocols/clock/clock.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"
#include "wingos-headers/startup.hpp"

// source: derived from brutal OS but
// I wrote the brutal PS2 code
int main(int, char **) { return 0; };

struct Waiter
{
    fc::Milliseconds start_time;
    fc::Milliseconds end_time;
    Wingos::MessageServerReceived msg;
};

int _main(StartupInfo *context)
{
    fc::Vec<Waiter> waiters = {};
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

    auto server_r = prot::ManagedServer::create_registered_server("clock", 1, 0);

    if (server_r.is_error())
    {
        fmt::err$("failed to create hio server: {}", server_r.error());
        return -1;
    }

    prot::ManagedServer server = fc::move(server_r.unwrap());

    fmt::log$("started clock service");
    while (true)
    {

        // fmt::log$("1 second to ms: {} ->{}", fc::Seconds(1).value(), fc::Seconds(1).to<fc::Milliseconds>().value());

        //        hw::hpet::hpet_sleep(fc::Seconds(1));
        //        fmt::log$("tick");

        server.accept_connection();

        for (long i = 0; i < (long)waiters.len(); i++)
        {
            auto &w = waiters[i];
            auto now = hw::hpet::hpet_clock_read();
            if (now >= w.end_time)
            {
                IpcMessage reply = {};
                reply.data[0].data = 0; // success
                server.reply(fc::move(w.msg), reply).unwrap();
                waiters.pop(i);
                i--;
            }
        }

        auto received = server.try_receive();
        if (!received.is_error())
        {
            auto msg = fc::move(received.unwrap());

            switch (msg.received.data[0].data)
            {
            case prot::CLOCK_GET_SYSTEM_TIME:
            {
                IpcMessage reply = {};
                fc::Milliseconds ms = hw::hpet::hpet_clock_read();
                //         fmt::log$("hpet: system time: {}ms", ms.value());
                reply.data[1].data = ms.value() / 1000;
                reply.data[2].data = ms.value();
                server.reply(fc::move(msg), reply).unwrap();
                break;
            }
            case prot::CLOCK_SLEEP_MS:
            {
                fc::Milliseconds ms = fc::Milliseconds(msg.received.data[1].data);

                auto start = hw::hpet::hpet_clock_read();
                auto end = start + ms;

                Waiter w = {};
                w.start_time = start;
                w.end_time = end;
                w.msg = fc::move(msg);
                waiters.push(fc::move(w));
                break;
            }
            default:
                fmt::warn$("hio: unknown message type received: {}", msg.received.data[0].data);
                break;
            }
        }
    }
}
