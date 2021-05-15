#pragma once
#include <stdint.h>

// https://wiki.osdev.org/RSDP
struct RSDP_descriptor_10
{
    char signature[8];
    uint8_t checksum;
    char OEM_signature[6];
    uint8_t rev;
    uint32_t RSDT_address;
} __attribute__((packed));

struct RSDPDescriptor20
{
    RSDP_descriptor_10 firstPart;

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct RSDTHeader
{
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__((packed));

struct RSDT
{
    RSDTHeader h;
    uint32_t PointerToOtherSDT[]; //
} __attribute__((packed));

class acpi
{
    uint64_t version = 0;
    RSDPDescriptor20 *descriptor;
    RSDT *rsdt_table;

public:
    acpi();
    void getFACP();
    void init_in_paging();

    void *find_entry(const char *entry_name);
    void descriptor_dump();
    void init();
    static acpi *the();
};
