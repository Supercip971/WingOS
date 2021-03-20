#include <arch.h>

#include <device/ahci.h>
#include <device/network/e1000.h>
#include <device/network/rtl8139.h>
#include <device/pci.h>
#include <logging.h>
#include <utils/liballoc.h>
pci_system main_system;
pci_device::pci_device(uint8_t bus, uint8_t device, uint8_t function)
{
    dbus = bus;
    ddev = device;
    dfunc = function;
}

bool pci_device::function_exist()
{
    if ((uint16_t)read_dword(0) != 0xffff)
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool pci_device::valid()
{
    return function_exist();
}

uint32_t pci_device::read_dword(uint8_t reg)
{
    // cast everything in int32
    uint32_t bus32 = (uint32_t)dbus;
    uint32_t device32 = (uint32_t)ddev;
    uint32_t function32 = (uint32_t)dfunc;

    uint32_t target = 0x80000000 | (bus32 << 16) | ((device32) << 11) | ((function32) << 8) | (reg & 0xFC);
    outl(0xcf8, target);

    return inl(0xCFC);
}
uint32_t pci_device::read_dword_func(uint8_t func, uint8_t reg)
{
    // cast everything in int32
    uint32_t bus32 = (uint32_t)dbus;
    uint32_t device32 = (uint32_t)ddev;
    uint32_t function32 = (uint32_t)func;

    uint32_t target = 0x80000000 | (bus32 << 16) | ((device32) << 11) | ((function32) << 8) | (reg & 0xFC);
    outl(0xcf8, target);

    return inl(0xCFC);
}

void pci_device::write_dword(uint8_t reg, uint32_t value)
{
    // cast everything in int32
    uint32_t bus32 = (uint32_t)dbus;
    uint32_t device32 = (uint32_t)ddev;
    uint32_t function32 = (uint32_t)dfunc;

    uint32_t target = (1 << 31) | (bus32 << 16) | ((device32 & 0b11111) << 11) | ((function32 & 0b111) << 8) | (reg & ~(0b11));
    outl(0xcf8, target);

    outl(0xCFC, value);
}
bool pci_device::is_bridge()
{
    if (get_header() == 0x1)
    {
        if (get_class() == 0x6)
        {
            return true;
        }
    }
    return false;
}

uint8_t pci_device::get_header()
{
    return (uint8_t)(((uint32_t)read_dword(0xc) >> 16) & ~(1 << 7));
}

uint8_t pci_device::get_sub_buss()
{

    return (uint8_t)(((uint32_t)read_dword(0x18) >> 8));
}

uint8_t pci_device::get_class()
{
    return (uint8_t)(((uint32_t)read_dword(0x8) >> 24));
}
uint8_t pci_device::get_irq()
{
    return (uint8_t)(((uint32_t)read_dword(0x3C)));
}

uint8_t pci_device::get_progif()
{
    return (uint8_t)(((uint32_t)read_dword(0x8) >> 8));
}

uint16_t pci_device::get_vendor()
{
    return ((uint16_t)(read_dword(0)));
}
uint16_t pci_device::get_dev_id()
{
    return (uint16_t)(((uint32_t)read_dword(0)) >> 16);
}
uint8_t pci_device::get_subclass()
{
    return (uint8_t)(((uint32_t)read_dword(0x8) >> 16));
}

bool pci_device::has_multiple_function()
{
    return ((uint8_t)((uint32_t)read_dword_func(0, 0xC) >> 16)) & (1 << 7);
}

pci_device_raw pci_device::to_raw()
{
    pci_device_raw r;
    r.bus = dbus;
    r.function = dfunc;
    r.device = ddev;
    return r;
}
void pci_device::enable_mastering()
{
    write_dword(0x4, read_dword(0x4) | 0x00000004);
}
pci_bar_data pci_device::get_bar(uint64_t id)
{
    if (get_header() == 2)
    {
        log("pci", LOG_WARNING) << "not supported header type";
    }
    uint64_t target = ((uint64_t)id * 4) + 0x10;
    pci_bar_data ret;
    uint32_t value = read_dword(target);
    uint32_t base = 0;
    //uint32_t type = value & 0b111;

    if ((value & 0b0111) == 0b0110)
    {
        ret.type = pci_bar_type::MM_IO_64;
        base = value & 0xFFFFFFF0;
    }
    else if ((value & 0b0111) == 0b0000)
    {
        ret.type = pci_bar_type::MM_IO_32;
        base = value & 0xFFFFFFF0;
    }
    else if ((value & 0b0111) == 0b0001)
    {
        ret.type = pci_bar_type::P_IO;
        base = value & 0xFFFFFFFC;
    }

    write_dword(target, 0xFFFFFFFF);
    uint64_t nvalue = read_dword(target);
    write_dword(target, value);

    ret.base = base;
    ret.size = ~(nvalue & 0xFFFFFFF0) + 1;

    log("pci", LOG_INFO) << "pci bar " << id << "func " << dfunc;
    log("pci", LOG_INFO) << "start" << ret.base;
    return ret;
}
pci_system::pci_system()
{
}

pci_system *pci_system::the()
{
    return &main_system;
}

void pci_system::scan_dev(uint8_t dev_id, uint8_t bus_id)
{
    pci_device device(bus_id, dev_id, 0);
    if (device.function_exist())
    {
        if (device.is_bridge())
        {
            log("pci", LOG_INFO) << "device : " << dev_id << "bus : " << bus_id << "is a bridge";
            scan_bus(device.get_sub_buss());
        }
        else
        {
            pci_devices[pci_devices_count] = device.to_raw();
            pci_devices_count++;
            if (device.has_multiple_function())
            {
                for (int i = 1; i < 8; i++)
                {
                    pci_device ndevice(bus_id, dev_id, i);
                    if (ndevice.function_exist())
                    {
                        if (ndevice.is_bridge())
                        {
                            log("pci", LOG_INFO) << "device : " << dev_id << "bus : " << bus_id << "is a bridge";

                            scan_bus(ndevice.get_sub_buss());
                        }
                        else
                        {

                            pci_devices[pci_devices_count] = ndevice.to_raw();
                            pci_devices_count++;
                        }
                    }
                }
            }
        }
    }
}

void pci_system::scan_bus(uint8_t bus_id)
{
    log("pci", LOG_INFO) << "scanning bus : " << bus_id;
    for (int i = 0; i < 32; i++)
    {
        scan_dev(i, bus_id);
    }
}

void pci_system::init_top_bus()
{
    log("pci", LOG_INFO) << "loading top bus";
    scan_bus(0);
}

void pci_system::init_device()
{
    log("pci", LOG_DEBUG) << "loading pci device";
    for (int i = 0; i < pci_devices_count; i++)
    {
        uint8_t dev_func = pci_devices[i].function;

        pci_device dev = pci_device(pci_devices[i].bus, pci_devices[i].device, dev_func);
        if (dev.get_vendor() == 0x8086)
        {
            if (dev.get_class() == 2 && dev.get_subclass() == 0)
            {

                uint16_t id = dev.get_dev_id();

                if (id == 0x100E || id == 0x153A || id == 0x10EA)
                {
                    dev.enable_mastering();
                    e1000::the()->init(&dev);
                }
            }
            else if (dev.get_class() == 1 && dev.get_subclass() == 6)
            {
                ahci *ahci_new_dev = new ahci(dev);
                ahci_new_dev->init();
            }
        }
        else if (dev.get_vendor() == 0x10EC)
        {
            if (dev.get_dev_id() == 0x8139)
            {
                dev.enable_mastering();
                rtl8139::the()->init(&dev);
            }
        }
    }
}
void pci_system::init()
{
    log("pci", LOG_DEBUG) << "loading pci";
    pci_devices = (pci_device_raw *)malloc(sizeof(pci_device_raw) * 128);
    pci_devices_count = 0;
    init_top_bus();
    log("pci", LOG_INFO) << "pci devices count : " << pci_devices_count;
    for (int i = 0; i < pci_devices_count; i++)
    {
        uint8_t dev_func = pci_devices[i].function;

        pci_device dev = pci_device(pci_devices[i].bus, pci_devices[i].device, dev_func);
        log("pci", LOG_INFO) << "=========";
        if (dev.get_vendor() == 0x8086)
        {
            log("pci", LOG_INFO) << "=intel=";
            if (dev.get_class() == (uint8_t)device_class_code::NETWORK_CONTROLLER && dev.get_subclass() == 0)
            {

                uint16_t id = dev.get_dev_id();

                if (id == 0x100E || id == 0x153A || id == 0x10EA)
                {
                    log("pci", LOG_INFO) << "==E1000==";
                }
            }
            else if (dev.get_class() == 1 && dev.get_subclass() == 6)
            {
                log("pci", LOG_INFO) << "AHCI";
            }
        }
        else if (dev.get_vendor() == 0x10EC)
        {
            log("pci", LOG_INFO) << "=realteck=";
            uint16_t id = dev.get_dev_id();
            if (id == 0x8139)
            {
                log("pci", LOG_INFO) << "==rtl8139==";
            }
        }
        log("pci", LOG_INFO) << "pci device = " << i;
        log("pci", LOG_INFO) << "bus        = " << pci_devices[i].bus;
        log("pci", LOG_INFO) << "device     = " << pci_devices[i].device;
        log("pci", LOG_INFO) << "function   = " << pci_devices[i].function;
        log("pci", LOG_INFO) << "vendor     = " << dev.get_vendor();
        log("pci", LOG_INFO) << "dev id     = " << dev.get_dev_id();
        log("pci", LOG_INFO) << "class      = " << dev.get_class();
        log("pci", LOG_INFO) << "irq        = " << dev.get_irq();
        log("pci", LOG_INFO) << "sub clas   = " << dev.get_subclass();
        log("pci", LOG_INFO) << "progif     = " << dev.get_progif();
        log("pci", LOG_INFO) << "device name= " << device_code_to_string(dev.get_class(), dev.get_subclass(), dev.get_progif());
    }
    init_device();
}
