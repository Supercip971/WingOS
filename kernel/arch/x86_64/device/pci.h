#pragma once
#include <stdint.h>
enum pci_bar_type
{
    MM_IO_32 = 0,
    MM_IO_64 = 1,
    P_IO = 2,
};

struct pci_bar_data
{
    uint64_t base;
    uint64_t io_base;
    uint32_t size;
    uint32_t type;
};

struct pci_device_raw
{
    uint8_t bus;
    uint8_t function;
    uint8_t device;
    int irq;
};
class pci_device
{
    uint8_t dbus;
    uint8_t ddev;
    uint8_t dfunc;

public:
    pci_device(uint8_t bus, uint8_t device, uint8_t function);
    void write_dword(uint8_t reg, uint32_t value);
    uint32_t read_dword(uint8_t reg);
    uint32_t read_dword_func(uint8_t func, uint8_t reg);
    bool function_exist();
    bool valid();
    uint8_t get_header();
    uint8_t get_class();
    uint16_t get_vendor();
    void enable_mastering();
    uint16_t get_dev_id();
    uint8_t get_sub_buss();
    uint8_t get_subclass();
    uint8_t get_progif();
    uint8_t get_irq();
    pci_device_raw to_raw();
    bool has_multiple_function();
    pci_bar_data get_bar(uint64_t id);
    bool is_bridge();
};

class pci_device_driver : public pci_device
{
public:
    pci_device_driver(uint8_t bus, uint8_t device, uint8_t function) : pci_device(bus, device, function){};
    pci_device_driver(pci_device d) : pci_device(d){};
};

class pci_system
{
    uint16_t pci_devices_count = 0;
    pci_device_raw *pci_devices; // max 128 device
public:
    pci_system();
    static pci_system *the();
    void scan_dev(uint8_t dev_id, uint8_t bus_id);
    void scan_bus(uint8_t bus_id);
    void init_top_bus();
    void init_device();
    void init();
};
