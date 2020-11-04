#pragma once
#include <stdint.h>
enum pci_bar_type
{
    MM_IO_32 = 0,
    MM_IO_64,
    P_IO,
};

struct pci_bar_data
{
    uint32_t base;
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

public:
    pci_device(uint8_t bus, uint8_t device);
    void write_dword(uint8_t func, uint8_t reg, uint32_t value);
    uint32_t read_dword(uint8_t func, uint8_t reg);
    bool function_exist(uint8_t function_id);
    bool valid();
    uint8_t get_header(uint8_t function);
    uint8_t get_class(uint8_t function);
    uint16_t get_vendor(uint8_t function);
    uint16_t get_dev_id(uint8_t function);
    uint8_t get_sub_buss(uint8_t function);
    uint8_t get_subclass(uint8_t function);
    uint8_t get_progif(uint8_t function);
    uint8_t get_irq(uint8_t function);
    pci_device_raw to_raw(uint8_t function);
    bool has_multiple_function();
    pci_bar_data get_bar(int id);
    bool is_bridge(uint8_t function);
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
    void init();
};
