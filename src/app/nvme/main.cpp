#include "arch/generic/syscalls.h"
#include "dev/pci/pci.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"




void setup_nvme_disk(Wingos::dev::PciDevice& dev)
{
    uint64_t device_addr = dev.get_bar64(Wingos::dev::pci_reg_dev_BAR0); // Get the BAR0 address

    log::log$("Setting up NVMe disk at address: {}", device_addr | fmt::FMT_HEX);
}

int _main(mcx::MachineContext*)
{

    // attempt connection 

    log::log$("hello world from nvme!");
    Wingos::dev::PciController pci_controller;
    pci_controller.scan_bus(0);


    for(auto &dev : pci_controller.devices)
    {
      
        if(dev.class_code() == 0x01 && dev.subclass() == 0x08) // storage controller, NVMe
        {
            log::log$("Found NVMe device: Bus {}, Device {}, Function {}, Vendor ID: {}, Device ID: {}",
                      dev.bus, dev.device, dev.function,
                      dev.vendor_id() | fmt::FMT_HEX, dev.device_id() | fmt::FMT_HEX);
            setup_nvme_disk(dev);
        }
    }


    while(true)
    {

        asm volatile("pause");
    }
    return 1;
}