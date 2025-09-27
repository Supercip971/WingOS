#include "dev/pci/pci.hpp"

#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"

namespace Wingos::dev
{

void PciController::scan_dev(uint8_t bus, uint8_t dev)
{
    PciDevice device = {
        .bus = bus,
        .device = dev,
        .function = 0,
        .irq = -1 // IRQ is not known yet
    };

    if (device.valid())
    {
        if (device.is_bridge())
        {
            scan_bus(device.sub_bus());
            return;
        }

        devices.push(device);

        if (device.has_multiple_functions())
        {
            for (uint8_t i = 1; i < 8; i++)
            {
                PciDevice sub_dev = {
                    .bus = bus,
                    .device = dev,
                    .function = i,
                    .irq = -1};

                if (!sub_dev.valid())
                {
                    continue;
                }

                if (sub_dev.is_bridge())
                {
                    scan_bus(sub_dev.sub_bus());
                }
                else
                {
                    devices.push(sub_dev);
                }
            }
        }
    }
}

void PciController::scan_bus(uint8_t bus)
{
    for (uint8_t dev = 0; dev < 32; dev++)
    {
        scan_dev(bus, dev);
    }
}

void PciController::dump()
{
    for (const auto &device : devices)
    {
        log::log$(
            "PCI Device: Bus {}, Device {}, Function {}, Vendor ID: {}, Device ID: {}, Class: {}, Subclass: {}",
            device.bus, device.device, device.function,
            device.vendor_id() | fmt::FMT_HEX, device.device_id() | fmt::FMT_HEX,
            device.class_code() | fmt::FMT_HEX, device.subclass() | fmt::FMT_HEX);
    }
}

} // namespace Wingos::dev