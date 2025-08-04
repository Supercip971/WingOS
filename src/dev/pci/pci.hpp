#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "iol/ports.hpp"
#include "libcore/ds/vec.hpp"
namespace Wingos::dev
{
constexpr size_t CONFIG_ADDRESS = 0xCF8;
constexpr size_t CONFIG_DATA = 0xCFC;

enum class PciBarType : uint8_t
{
    MEM_IO_32 = 0x00, // 32-bit memory-mapped I/O
    MEM_IO_64 = 0x01, // 64-bit memory-mapped I/O
    PORT_IO = 0x02,   // Prefetchable memory-mapped
};

struct PciBarData
{
    uint64_t base;
    uint64_t io_base;
    uint32_t size;
    uint32_t type;
};

struct [[gnu::packed]] PciConfigReg
{
    uint8_t offset;
    uint8_t function : 3;
    uint8_t device : 5;
    uint8_t bus;
    uint8_t _reserved : 7;
    uint8_t enable : 1; // 1 to enable, 0 to disable
};
static_assert(sizeof(PciConfigReg) == 4, "PciConfigReg must be 4 bytes");

constexpr uint32_t pci_register_0 = 0;
struct [[gnu::packed]] PciReg0
{
    uint16_t vendor_id; // 0x00
    uint16_t device_id; // 0x02
};

constexpr uint32_t pci_register_1 = 0x4;
struct [[gnu::packed]] PciReg1
{
    uint16_t command; // 0x04
    uint16_t status;  // 0x06
};

constexpr uint32_t pci_register_2 = 0x8;
struct [[gnu::packed]] PciReg2
{
    uint8_t revision_id; // 0x08
    uint8_t prog_if;     // 0x09
    uint8_t subclass;    // 0x0A
    uint8_t class_code;  // 0x0B
};

constexpr uint32_t pci_register_3 = 0xc;
struct [[gnu::packed]] PciReg3
{
    uint8_t cache_line_size; // 0x0C
    uint8_t latency_timer;   // 0x0D
    uint8_t header_type;     // 0x0E
    uint8_t bist;            // 0x0F
};

enum PciHeaderType : uint8_t
{
    HEADER_TYPE_NORMAL = 0x00,            // Normal device
    HEADER_TYPE_PCI_TO_PCI_BRIDGE = 0x01, // PCI-to-PCI bridge
    HEADER_TYPE_CARDBUS_BRIDGE = 0x02,    // CardBus bridge

};

constexpr uint32_t pci_reg_dev_BAR0 = 0x10;
constexpr uint32_t pci_reg_dev_BAR1 = 0x14;
constexpr uint32_t pci_reg_dev_BAR2 = 0x18;
constexpr uint32_t pci_reg_dev_BAR3 = 0x1C;
constexpr uint32_t pci_reg_dev_BAR4 = 0x20;
constexpr uint32_t pci_reg_dev_BAR5 = 0x24;

constexpr uint32_t pci_reg_bridge_BAR0 = 0x10;
constexpr uint32_t pci_reg_bridge_BAR1 = 0x14;

constexpr uint32_t pci_reg_bridge_secondary_bus = 0x18;
struct [[gnu::packed]] PciBridgeRegSecBus
{

    uint8_t primary_bus_number; // 0x3B

    uint8_t secondary_bus_number;    // 0x3A
    uint8_t subordinate_bus_number;  // 0x39
    uint8_t secondary_latency_timer; // 0x38
};

constexpr uint32_t pci_reg_bridge_io_base_stat = 0x1C;
struct [[gnu::packed]] PciBridgeRegIoBaseStat
{
    uint8_t io_base;           // 0x1C
    uint8_t io_limit;          // 0x1D
    uint16_t secondary_status; // 0x1E
};

constexpr uint32_t pci_reg_bridge_mem_base_stat = 0x20;
struct [[gnu::packed]] PciBridgeRegMemBaseStat
{
    uint16_t mem_base;  // 0x20
    uint16_t mem_limit; // 0x24
};

struct PciDevice
{
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    int irq;

    static inline uint32_t read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t off)
    {
        PciConfigReg reg = {off, func, slot, bus, 0, 1};
        uint32_t address = *(uint32_t *)&reg;

        // Write the address to the CONFIG_ADDRESS port

        iol::outl(CONFIG_ADDRESS, address);
        // Read the data from the CONFIG_DATA port
        return iol::inl(CONFIG_DATA);
    }

    uint32_t read_config(uint8_t off) const
    {
        return read_config(bus, device, function, off);
    }
    uint32_t read_config(uint8_t off, size_t func) const
    {
        return read_config(bus, device, func, off);
    }

    uint32_t get_bar(uint8_t bar_index) const
    {
        switch (bar_index)
        {
        case 0:
            return read_config(pci_reg_dev_BAR0);
        case 1:
            return read_config(pci_reg_dev_BAR1);
        case 2:
            return read_config(pci_reg_dev_BAR2);
        case 3:
            return read_config(pci_reg_dev_BAR3);
        case 4:
            return read_config(pci_reg_dev_BAR4);
        case 5:
            return read_config(pci_reg_dev_BAR5);
        default:
            return 0;
        }
    }

    uint64_t get_bar64(uint8_t bar_index) const
    {
        uint32_t low = read_config(bar_index) & ~(uint32_t)0xFFF; // Clear the lower 12 bits for alignment
        uint32_t high = read_config(bar_index + 4);
        return ((uint64_t)high << 32) | low;
    }

    static inline void write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t off, uint32_t value)
    {
        PciConfigReg reg = {off, func, slot, bus, 0, 1};
        uint32_t address = *(uint32_t *)&reg;

        // Write the address to the CONFIG_ADDRESS port
        iol::outl(CONFIG_ADDRESS, address);
        // Write the data to the CONFIG_DATA port
        iol::outl(CONFIG_DATA, value);
    }

    void write_config(uint8_t off, uint32_t value) const
    {
        write_config(bus, device, function, off, value);
    }

    template <typename T>
    T read_config(size_t reg) const
    {
        static_assert(sizeof(T) <= 4, "Only 32-bit reads are supported");
        uint32_t value = read_config(reg);
        return *(T *)&value;
    }

    template <typename T>
    T read_config(size_t reg, size_t func) const
    {
        static_assert(sizeof(T) <= 4, "Only 32-bit reads are supported");
        uint32_t value = read_config(reg, func);
        return *(T *)&value;
    }

    uint16_t device_id() const
    {
        return read_config<PciReg0>(pci_register_0).device_id;
    }

    uint16_t vendor_id() const
    {
        return read_config<PciReg0>(pci_register_0).vendor_id;
    }

    uint16_t command() const
    {
        return read_config<PciReg1>(pci_register_1).command;
    }

    uint16_t status() const
    {
        return read_config<PciReg1>(pci_register_1).status;
    }

    uint8_t revision_id() const
    {
        return read_config<PciReg2>(pci_register_2).revision_id;
    }

    uint8_t prog_if() const
    {
        return read_config<PciReg2>(pci_register_2).prog_if;
    }

    uint8_t subclass() const
    {
        return read_config<PciReg2>(pci_register_2).subclass;
    }

    uint8_t class_code() const
    {
        return read_config<PciReg2>(pci_register_2).class_code;
    }

    bool valid() const
    {
        return vendor_id() != 0xFFFF && device_id() != 0xFFFF;
    }

    uint8_t header() const
    {
        return read_config<PciReg3>(pci_register_3).header_type & ~(0x80);
    }
    bool is_bridge() const
    {
        return header() == PciHeaderType::HEADER_TYPE_PCI_TO_PCI_BRIDGE &&
               class_code() == 0x6;
    }

    uint8_t sub_bus() const
    {
        return read_config<PciBridgeRegSecBus>(pci_reg_bridge_secondary_bus).secondary_bus_number;
    }

    bool has_multiple_functions() const
    {
        return read_config<PciReg3>(pci_register_3, 0).header_type & 0x80;
    }
};

struct PciController
{
    core::Vec<PciDevice> devices;

public:
    void scan_dev(uint8_t bus, uint8_t dev);
    void scan_bus(uint8_t bus);

    void scan()
    {
        scan_bus(0);
    }

    void dump();
};
}; // namespace Wingos::dev