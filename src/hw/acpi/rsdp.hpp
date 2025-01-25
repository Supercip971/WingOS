
#pragma once

#include <stdint.h>

#include "hw/mem/addr_space.hpp"

namespace hw::acpi
{
struct [[gnu::packed]] RsdpExtended
{

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
};

enum class RsdtTypes
{
    RSDT,
    XSDT
};

struct RsdtRet
{
    PhysAddr physical_addr;
    RsdtTypes type;

    RsdtRet() = default;
    RsdtRet(PhysAddr addr, RsdtTypes type) : physical_addr(addr), type(type) {};
};

struct [[gnu::packed]] Rsdp
{
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;

    RsdpExtended v2_extension;

    int rsdp_version() const
    {
        return this->revision;
    };

    bool check() const
    {
        const char *expected_signature = "RSD PTR ";
        for (int i = 0; i < 8; i++)
        {
            if (this->signature[i] != expected_signature[i])
            {
                return false;
            }
        }

        return true;
    }

    RsdtRet rsdt_phys_addr() const
    {
        // from spec: If the revision is 2 or greater, the physical address of the XSDT is stored in the 64-bit field.
        // and it should be used instead of the old 32 bit address.

        if (this->rsdp_version() >= 2)
        {
            return RsdtRet(PhysAddr(this->v2_extension.xsdt_address), RsdtTypes::XSDT);
        }

        return RsdtRet(PhysAddr(this->rsdt_address), RsdtTypes::RSDT);
    }
};
}; // namespace hw::acpi