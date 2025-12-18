#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include "hw/mem/addr_space.hpp"
#include "protocols/server_helper.hpp"

#include "hw/acpi/rsdt.hpp"
#include "hw/hpet/hpet.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/time/time.hpp"
#include "math/align.hpp"
#include "math/range.hpp"
#include "protocols/clock/clock.hpp"
#include "protocols/pipe/pipe.hpp"
#include "wingos-headers/ipc.h"
#include "wingos-headers/startup.hpp"

// source: derived from brutal OS but
// I wrote the brutal PS2 code
int main(int , char **){return 0;};


struct Waiter
{
    core::Milliseconds start_time;
    core::Milliseconds end_time;
    Wingos::MessageServerReceived msg;
};


int _main(StartupInfo*context)
{
    core::Vec<Waiter> waiters = {};
    log::log$("clock from rsdp addr: {}", context->machine_context_optional._rsdp | fmt::FMT_HEX);

    uintptr_t phys_rsdp = context->machine_context_optional._rsdp - 0xffff800000000000;



    log::log$("preparing rsdp mapping");
    hw::acpi::prepare_mapping(phys_rsdp, [](uintptr_t addr, size_t size) {
        auto vrange = math::Range<size_t>(addr, addr + size).growAlign(4096);

        log::log$("mapping hpet rsdp region: {} - {}", vrange.start() | fmt::FMT_HEX, vrange.end() | fmt::FMT_HEX);

        Wingos::Space::self().map_physical_memory(vrange.start(), vrange.len(), ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
        return core::Result<size_t>{
            vrange.start()
        };
    }).unwrap();

    log::log$("preparing hpet");

    hw::hpet::hpet_prepare_mapping(phys_rsdp, [](uintptr_t addr, size_t size) {
        auto vrange = math::Range<size_t>(addr, addr + size).growAlign(4096);

        log::log$("mapping hpet region: {} - {}", vrange.start() | fmt::FMT_HEX, vrange.end() | fmt::FMT_HEX);
        Wingos::Space::self().map_physical_memory(vrange.start(), vrange.len(), ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
        return core::Result<size_t>{
            vrange.start()
        };
    }).unwrap();

    hw::hpet::hpet_initialize(toVirt(phys_rsdp).as<hw::acpi::Rsdp>()).unwrap();
    // map everything



    auto server_r = prot::ManagedServer::create_registered_server("clock", 1, 0);

    if(server_r.is_error())
    {
        log::err$("failed to create hio server: {}", server_r.error());
        return -1;
    }

    prot::ManagedServer server = core::move(server_r.unwrap());

    log::log$("started clock service");
    while (true)
    {

        //log::log$("1 second to ms: {} ->{}", core::Seconds(1).value(), core::Seconds(1).to<core::Milliseconds>().value());

//        hw::hpet::hpet_sleep(core::Seconds(1));
//        log::log$("tick");

        server.accept_connection();

        for (long i = 0; i < (long)waiters.len(); i++)
        {
            auto &w = waiters[i];
            auto now = hw::hpet::hpet_clock_read();
            if (now >= w.end_time)
            {
                IpcMessage reply = {};
                reply.data[0].data = 0; // success
                server.reply(core::move(w.msg), reply).unwrap();
                waiters.pop(i);
                i--;
            }
        }


        auto received = server.try_receive();
        if (!received.is_error())
        {
            auto msg = core::move(received.unwrap());

            switch (msg.received.data[0].data)
            {
            case prot::CLOCK_GET_SYSTEM_TIME:
            {
                IpcMessage reply = {};
                core::Milliseconds ms = hw::hpet::hpet_clock_read();
       //         log::log$("hpet: system time: {}ms", ms.value());
                reply.data[1].data = ms.value()/1000;
                reply.data[2].data = ms.value();
                server.reply(core::move(msg), reply).unwrap();
                break;
            }
            case prot::CLOCK_SLEEP_MS:
            {
                core::Milliseconds ms = core::Milliseconds(msg.received.data[1].data);

                auto start = hw::hpet::hpet_clock_read();
                auto end = start + ms;

                Waiter w = {};
                w.start_time = start;
                w.end_time = end;
                w.msg = core::move(msg);
                waiters.push(core::move(w));
                break;
            }
            default:
                log::warn$("hio: unknown message type received: {}", msg.received.data[0].data);
                break;
            }
        }
    }
}
