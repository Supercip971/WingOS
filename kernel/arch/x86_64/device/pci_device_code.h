#ifndef PCI_STR_CONVERT_H
#define PCI_STR_CONVERT_H

#include <stdint.h>
enum class device_class_code : uint8_t
{
    MASS_STORAGE_CONTROLLER = 1,
    NETWORK_CONTROLLER = 2,
    DISPLAY_CONTROLLER = 3,
    MULTIMEDIA_CONTROLLER = 4,
    MEMORY_CONTROLLER = 5,
    BRIDGE_DEVICE = 6,
    BASE_SYSTEM_PERIPHERAL = 8,
    SERIAL_BUS_CONTROLLER = 0xC,
};

const char *device_code_to_string(uint8_t class_code, uint8_t subclass, uint8_t prog_if);

// -------------------------- MASS_STORAGE_CONTROLLER --------------------------

enum class mass_storage_device_subclass : uint8_t
{
    SCSI_BUS_CONTROLLER = 0,
    IDE_CONTROLLER = 1,
    FLOPPY_DISK_CONTROLLER = 2,
    IPI_BUS_CONTROLLER = 3,
    RAID_CONTROLLER = 4,
    ATA_CONTROLLER = 5,
    SATA_CONTROLLER = 6,
    SERIAL_SCSI_CONTROLLER = 7,
    NVM_CONTROLLER = 8,
};

enum class mass_storage_device_SATA : uint8_t
{
    SPECIFIC_SATA_CONTROLLER = 0,
    AHCI_SATA_CONTROLLER = 1,
    SERIAL_SATA_CONTROLLER = 2,
};

enum class mass_storage_device_NVM : uint8_t
{
    NVMHCI_CONTROLLER = 1,
    NVMe_CONTROLLER = 2,
};

const char *mass_storage_device_str(uint8_t subclass, uint8_t prog_if);

// -------------------------- NETWORK_CONTROLLER --------------------------

enum class network_device_subclass : uint8_t
{
    ETHERNET_CONTROLLER = 0,
    TOKEN_RING_CONTROLLER = 1,
    FDDI_CONTROLLER = 2,
    ATM_CONTROLLER = 3,
    ISDN_CONTROLLER = 4,
    WorldFip_CONTROLLER = 5,
    PICMG_CONTROLLER = 6,
    INFINIBAND_CONTROLLER = 7,
    FABRIC_CONTROLLER = 8,
};

const char *network_device_str(uint8_t subclass, uint8_t prog_if);

// -------------------------- DISPLAY_CONTROLLER --------------------------

enum class display_controller_subclass : uint8_t
{
    VGA_CONTROLLER = 0,
    XGA_CONTROLLER = 1,
    D3D_CONTROLLER = 2,
};

const char *display_device_str(uint8_t subclass, uint8_t prog_if);

// -------------------------- BRIDGE_DEVICE --------------------------

enum class bridge_device_subclass : uint8_t
{
    HOST_BRIDGE = 0,
    ISA_BRIDGE = 1,
    EISA_BRIDGE = 2,
    MCA_BRIDGE = 3,
    PCI2PCI_BRIDGE = 4,
    PCMCIA_BRIDGE = 5,
    NUBUS_BRIDGE = 6,
    CARDBUS_BRIDGE = 7,
    RACEWAY_BRIDGE = 8,
    SEMIT_RANSPARENT_PCI2PCI_BRIDGE = 9,
    INFINIBAND2PCI_HOST_BRIDGE = 10,
};

const char *bridge_device_str(uint8_t subclass, uint8_t prog_if);

// -------------------------- BASE_SYSTEM_PERIPHERAL --------------------------

enum class base_system_peripheral_subclass : uint8_t
{
    INTERRUPT_CONTROLLER = 0,
    DMA_CONTROLLER = 1,
    INTERRUPT_TIMER_CONTROLLER = 2,
    RTC_CONTROLLER = 3,
    PCI_HOT_PLUG_CONTROLLER = 4,
    SDHCI = 5,
    IOMMU = 6,
};

enum class base_system_peripheral_int_controller : uint8_t
{
    PIC_8259_COMPATIBLE = 0,
    PIC_ISA_COMPATIBLE = 1,
    PIC_EISA_COMPATIBLE = 2,
    IOAPIC_IRQ_CONTROLLER = 0x10,
    IOxAPIC_IRQ_CONTROLLER = 0x20,
};

enum class base_system_peripheral_dma_controller : uint8_t
{
    DMA_8237_COMPATIBLE = 0,
    DMA_ISA_COMPATIBLE = 1,
    DMA_EISA_COMPATIBLE = 2,
};

enum class base_system_peripheral_timer_controller : uint8_t
{
    PIT_8237_COMPATIBLE = 0,
    PIT_ISA_COMPATIBLE = 1,
    PIT_EISA_COMPATIBLE = 2,
    HPET = 3,
};

const char *base_sys_device_str(uint8_t subclass, uint8_t prog_if);

// -------------------------- SERIAL_BUS_CONTROLLER --------------------------

enum class serial_bus_controller_subclass : uint8_t
{
    FIREWIRE_CONTROLLER = 0,
    ACCESS_BUS_CONTROLLER = 1,
    SSA_CONTROLLER = 2,
    USB_CONTROLLER = 3,
    FIBRE_CHANNEL_CONTROLLER = 4,
    SMBus_CONTROLLER = 5,
    InfiniBand_CONTROLLER = 6,
    IMPMI_INTERFACE_CONTROLLER = 7,
};

enum class serial_firewire_controller : uint8_t
{
    GENERIC_FIREWIRE_CONTROLLER = 0,
    OHCI_FIREWIRE_CONTROLLER = 0x10,
};

enum class serial_usb_controller : uint8_t
{
    uHCI_USB1_CONTROLLER = 0,
    oHCI_USB1_CONTROLLER = 0x10,
    eHCI_USB2_CONTROLLER = 0x20,
    xHCI_USB3_CONTROLLER = 0x30,
    USB_DEVICE = 0xFE,
};

const char *serial_bus_device_str(uint8_t subclass, uint8_t prog_if);

#endif // PCI_STR_CONVERT_H
