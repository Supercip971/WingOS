#pragma once

#include "libcore/fmt/log.hpp"
#include "libcore/str.hpp"
namespace Wingos::dev
{
constexpr core::Str dev_classes[] = {
    "unknown",
    "storage",
    "network",
    "display",
    "multimedia",
    "memory",
    "bridge",
    "communication controller",
    "generic system peripheral",
    "input device",
    "docking",
    "cpu",
    "serial",
    "wireless",
    "intelligent",
    "satellite",
    "encryption",
    "signal processing",
    "accelerators",
    "non-essential",
};

constexpr size_t dev_classes_count = sizeof(dev_classes) / sizeof(core::Str);

constexpr core::Str dev_classes_storage[] = {
    "scsi",
    "IDE",
    "floppy",
    "ipi",
    "raid",
    "ATA",
    "SATA",
    "SASCSI",
    "NVMe",
    "Universal Flash Storage",

};

constexpr size_t dev_classes_storage_count = sizeof(dev_classes_storage) / sizeof(core::Str);

constexpr core::Str dev_classes_network[] = {
    "ethernet",
    "token ring",
    "FDDI",
    "ATM",
    "ISDN",
    "WorldFIP",
    "PICMG",
    "InfiniBand",
    "Fabric",
};

constexpr size_t dev_classes_network_count = sizeof(dev_classes_network) / sizeof(core::Str);

constexpr core::Str dev_classes_display[] = {
    "VGA",
    "XGA",
    "3D",
};

constexpr size_t dev_classes_display_count = sizeof(dev_classes_display) / sizeof(core::Str);

static inline void log_dev(uint8_t class_code, uint8_t subclass)
{
    if (class_code >= dev_classes_count)
    {
        log::log$("Unknown device class: {}", class_code);
        return;
    }

    core::Str class_str = dev_classes[class_code];

    if (class_code == 0x01) // storage
    {
        if (subclass < dev_classes_storage_count)
        {
            log::log$("Storage device: {}", dev_classes_storage[subclass]);
            return;
        }
    }
    else if (class_code == 0x02) // network
    {
        if (subclass < dev_classes_network_count)
        {
            log::log$("Network device: {}", dev_classes_network[subclass]);
            return;
        }
    }
    else if (class_code == 0x03) // display
    {
        if (subclass < dev_classes_display_count)
        {
            log::log$("Display device: {}", dev_classes_display[subclass]);
            return;
        }
    }

    log::log$("Unknown device class: {}, subclass: {}", class_str, subclass);
}
} // namespace Wingos::dev