#include "pci_device_code.h"

const char *mass_storage_device_str(uint8_t subclass, uint8_t prog_if)
{

    switch ((mass_storage_device_subclass)subclass)
    {

    case mass_storage_device_subclass::SCSI_BUS_CONTROLLER:
        return "scsi bus controller";

    case mass_storage_device_subclass::IDE_CONTROLLER:
        return "ide controller";

    case mass_storage_device_subclass::FLOPPY_DISK_CONTROLLER:
        return "floppy disk controller";

    case mass_storage_device_subclass::IPI_BUS_CONTROLLER:
        return "ipi bus controller";

    case mass_storage_device_subclass::RAID_CONTROLLER:
        return "raid controller";

    case mass_storage_device_subclass::ATA_CONTROLLER:
        return "ata controller";

    case mass_storage_device_subclass::SATA_CONTROLLER:
        switch ((mass_storage_device_SATA)prog_if)
        {
        case mass_storage_device_SATA::AHCI_SATA_CONTROLLER:
            return "ahci sata controller";

        case mass_storage_device_SATA::SPECIFIC_SATA_CONTROLLER:
            return "specific sata controller";

        case mass_storage_device_SATA::SERIAL_SATA_CONTROLLER:
            return "serial sata controller";

        default:
            return "unknown sata controller";
        }
    case mass_storage_device_subclass::SERIAL_SCSI_CONTROLLER:
        return "serial scsi controller";

    case mass_storage_device_subclass::NVM_CONTROLLER:
        switch ((mass_storage_device_NVM)prog_if)
        {
        case mass_storage_device_NVM::NVMHCI_CONTROLLER:
            return "nvmhci controller";

        case mass_storage_device_NVM::NVMe_CONTROLLER:
            return "nvme controller";

        default:
            return "unknown nvm controller";
        }

    default:
        return "unknown mass storage controller";
    }
}

const char *network_device_str(uint8_t subclass, uint8_t prog_if)
{
    switch ((network_device_subclass)subclass)
    {

    case network_device_subclass::ETHERNET_CONTROLLER:
        return "ethernet controller";

    case network_device_subclass::TOKEN_RING_CONTROLLER:
        return "token ring controller";

    case network_device_subclass::FDDI_CONTROLLER:
        return "fddi controller";

    case network_device_subclass::ATM_CONTROLLER:
        return "atm controller";

    case network_device_subclass::ISDN_CONTROLLER:
        return "isdn controller";

    case network_device_subclass::WorldFip_CONTROLLER:
        return "worldfip controller";

    case network_device_subclass::PICMG_CONTROLLER:
        return "picmg controller";

    case network_device_subclass::INFINIBAND_CONTROLLER:
        return "infiniband controller";

    case network_device_subclass::FABRIC_CONTROLLER:
        return "fabric controller";
    default:
        return "unknown network controller";
    }
}

const char *display_device_str(uint8_t subclass, uint8_t prog_if)
{
    switch ((display_controller_subclass)subclass)
    {

    case display_controller_subclass::VGA_CONTROLLER:
        return "VGA controller";

    case display_controller_subclass::XGA_CONTROLLER:
        return "XGA controller";

    case display_controller_subclass::D3D_CONTROLLER:
        return "3D controller";

    default:
        return "unknown display controller";
    }
}

const char *bridge_device_str(uint8_t subclass, uint8_t prog_if)
{
    switch ((bridge_device_subclass)subclass)
    {

    case bridge_device_subclass::HOST_BRIDGE:
        return "Host bridge";

    case bridge_device_subclass::ISA_BRIDGE:
        return "isa bridge";

    case bridge_device_subclass::EISA_BRIDGE:
        return "eisa bridge";

    case bridge_device_subclass::MCA_BRIDGE:
        return "mca bridge";

    case bridge_device_subclass::PCI2PCI_BRIDGE:
        return "pci to pci bridge";

    case bridge_device_subclass::PCMCIA_BRIDGE:
        return "pcimcia bridge";

    case bridge_device_subclass::NUBUS_BRIDGE:
        return "nubus bridge";

    case bridge_device_subclass::CARDBUS_BRIDGE:
        return "cardbus bridge";

    case bridge_device_subclass::RACEWAY_BRIDGE:
        return "raceway bridge";

    case bridge_device_subclass::SEMIT_RANSPARENT_PCI2PCI_BRIDGE:
        return "semi-transparent pci to pci bridge";

    case bridge_device_subclass::INFINIBAND2PCI_HOST_BRIDGE:
        return "infiniband to pci bridge";

    default:
        return "unknown bridge controller";
    }
}
const char *base_sys_device_str(uint8_t subclass, uint8_t prog_if)
{
    switch ((base_system_peripheral_subclass)subclass)
    {

    case base_system_peripheral_subclass::INTERRUPT_CONTROLLER:
        switch ((base_system_peripheral_int_controller)prog_if)
        {
        case base_system_peripheral_int_controller::PIC_8259_COMPATIBLE:
            return "pic 8259 compatible controller";

        case base_system_peripheral_int_controller::PIC_ISA_COMPATIBLE:
            return "pic isa compatible controller";

        case base_system_peripheral_int_controller::PIC_EISA_COMPATIBLE:
            return "pic eisa compatible controller";

        case base_system_peripheral_int_controller::IOAPIC_IRQ_CONTROLLER:
            return "io apic irq controller";

        case base_system_peripheral_int_controller::IOxAPIC_IRQ_CONTROLLER:
            return "io xapic irq controller";

        default:
            return "unknown base system interrupt controller";
        }

    case base_system_peripheral_subclass::DMA_CONTROLLER:
        switch ((base_system_peripheral_dma_controller)prog_if)
        {
        case base_system_peripheral_dma_controller::DMA_8237_COMPATIBLE:
            return "dma 8237 compatible controller";

        case base_system_peripheral_dma_controller::DMA_ISA_COMPATIBLE:
            return "dma isa compatible controller";

        case base_system_peripheral_dma_controller::DMA_EISA_COMPATIBLE:
            return "dma eisa compatible controller";

        default:
            return "unknown base system dma controller";
        }

    case base_system_peripheral_subclass::INTERRUPT_TIMER_CONTROLLER:
        switch ((base_system_peripheral_timer_controller)prog_if)
        {
        case base_system_peripheral_timer_controller::PIT_8237_COMPATIBLE:
            return "PIT 8237 compatible controller";

        case base_system_peripheral_timer_controller::PIT_ISA_COMPATIBLE:
            return "PIT isa controller";

        case base_system_peripheral_timer_controller::PIT_EISA_COMPATIBLE:
            return "PIT eisa controller";

        case base_system_peripheral_timer_controller::HPET:
            return "hpet controller";

        default:
            return "unknown base system interrupt timer controller";
        }

    case base_system_peripheral_subclass::RTC_CONTROLLER:
        return "rtc controller";

    case base_system_peripheral_subclass::PCI_HOT_PLUG_CONTROLLER:
        return "pci hotplug controller";

    case base_system_peripheral_subclass::SDHCI:
        return "sdhci controller";

    case base_system_peripheral_subclass::IOMMU:
        return "iommu controller";

    default:
        return "unknown base system peripheral";
    }
}

const char *serial_bus_device_str(uint8_t subclass, uint8_t prog_if)
{
    switch ((serial_bus_controller_subclass)subclass)
    {

    case serial_bus_controller_subclass::FIREWIRE_CONTROLLER:
        switch ((serial_firewire_controller)prog_if)
        {
        case serial_firewire_controller::GENERIC_FIREWIRE_CONTROLLER:
            return "generic firewire controller";

        case serial_firewire_controller::OHCI_FIREWIRE_CONTROLLER:
            return "oHCI firewire controller";

        default:
            return "unknown firewire controller";
        }

    case serial_bus_controller_subclass::ACCESS_BUS_CONTROLLER:
        return "access bus controller";

    case serial_bus_controller_subclass::SSA_CONTROLLER:
        return "ssa controller";

    case serial_bus_controller_subclass::USB_CONTROLLER:
        switch ((serial_usb_controller)prog_if)
        {
        case serial_usb_controller::uHCI_USB1_CONTROLLER:
            return "uHCI usb1 controller";

        case serial_usb_controller::oHCI_USB1_CONTROLLER:
            return "oHCI usb1 controller";

        case serial_usb_controller::eHCI_USB2_CONTROLLER:
            return "eHCI usb2 controller";

        case serial_usb_controller::xHCI_USB3_CONTROLLER:
            return "xHCI usb3 controller";

        case serial_usb_controller::USB_DEVICE:
            return "usb device";

        default:
            return "unknown usb controller";
        }

    case serial_bus_controller_subclass::FIBRE_CHANNEL_CONTROLLER:
        return "fibre channel controller";

    case serial_bus_controller_subclass::SMBus_CONTROLLER:
        return "SMbus controller";

    case serial_bus_controller_subclass::InfiniBand_CONTROLLER:
        return "Infiniband controller";
    case serial_bus_controller_subclass::IMPMI_INTERFACE_CONTROLLER:
        return "IMPMI controller";

    default:
        return "unknown serial bus controller";
    }
}

const char *device_code_to_string(uint8_t class_code, uint8_t subclass, uint8_t prog_if)
{
    switch ((device_class_code)class_code)
    {
    case device_class_code::MASS_STORAGE_CONTROLLER:
        return mass_storage_device_str(subclass, prog_if);

    case device_class_code::NETWORK_CONTROLLER:
        return network_device_str(subclass, prog_if);

    case device_class_code::DISPLAY_CONTROLLER:
        return display_device_str(subclass, prog_if);

    case device_class_code::MULTIMEDIA_CONTROLLER:
        return "multimedia controller";

    case device_class_code::MEMORY_CONTROLLER:
        return "memory controller";

    case device_class_code::BRIDGE_DEVICE:
        return bridge_device_str(subclass, prog_if);

    case device_class_code::BASE_SYSTEM_PERIPHERAL:
        return base_sys_device_str(subclass, prog_if);

    case device_class_code::SERIAL_BUS_CONTROLLER:
        return serial_bus_device_str(subclass, prog_if);

    default:
        return "unknown";
    }
}
