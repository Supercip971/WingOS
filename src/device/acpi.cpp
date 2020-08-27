#include <arch/mem/virtual.h>
#include <com.h>
#include <device/acpi.h>
#include <utility.h>
acpi main_acpi;

void *get_rsdp(void)
{
    for (int i = get_mem_addr(0x80000); i < get_mem_addr(0x100000); i += 16)
    {
        if (i == get_mem_addr(0xa0000))
        {
            i = get_mem_addr(0xe0000 - 16);
            continue;
        }
        if (!strncmp((char *)i, "RSD PTR ", 8))
        {
            com_write_reg("acpi: Found RSDP at ", i);
            return (void *)i;
        }
    }

    return 0x0;
}
acpi::acpi()
{
}


void *acpi::find_entry(const char* entry_name){
    RSDT *rsdt = (RSDT *)descriptor->firstPart.RSDT_address;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        com_write_str("searching ...");

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            com_write_str("skipping entry 0");
            continue;
        }
        RSDTHeader *h = (RSDTHeader *)(rsdt->PointerToOtherSDT[i]);

        if (!strncmp(h->Signature, entry_name, 4))
            return (void *)h;
    }

    // Not found
    return 0x0;
}
void *findFACP(void *RootSDT)
{
    RSDT *rsdt = (RSDT *)RootSDT;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        com_write_str("searching ...");

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            com_write_str("skipping entry 0");
            continue;
        }
        RSDTHeader *h = (RSDTHeader *)(rsdt->PointerToOtherSDT[i]);

        if (!strncmp(h->Signature, "FACP", 4))
            return (void *)h;
    }

    // No FACP found
    return 0x0;
}
void acpi::init(uint64_t rsdp)
{
    com_write_str("acpi 1");
    descriptor = (RSDPDescriptor20 *)(((uint64_t)get_rsdp()));
    com_write_str("acpi 2");
    rsdt_table = (RSDT *)(descriptor->firstPart.RSDT_address);
    com_write_str("acpi 3");
}
void acpi::init_in_paging()
{

    RSDT *rsdt = rsdt_table;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
    uint64_t ddat = (uint64_t)rsdt;
    ddat /= 4096;
    ddat *= 4096;
    virt_map(ddat, get_mem_addr(ddat), 0x03);
    virt_map(ddat + 4096, get_mem_addr(ddat + 4096), 0x03);
    rsdt = (RSDT *)get_mem_addr((uint64_t)rsdt);
    for (int i = 0; i < entries; i++)
    {
        uint64_t addr = rsdt->PointerToOtherSDT[i];
        addr /= 4096;
        addr -= 1;
        addr *= 4096;
        virt_map(addr, get_mem_addr(addr), 0x03);
        virt_map(addr + 4096, get_mem_addr(addr + 4096), 0x03);
        virt_map(addr + (4096 * 2), get_mem_addr(addr + (4096 * 2)), 0x03);
    }
}
void acpi::getFACP()
{
    RSDT *rsdt = (RSDT *)rsdt_table;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        com_write_str("searching ...");

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            com_write_str("skipping entry 0");
            continue;
        }
        RSDTHeader *h = (RSDTHeader *)(rsdt->PointerToOtherSDT[i]);

        com_write_str(h->Signature);
    }
    com_write_reg("FACP : ", (uint64_t)findFACP(rsdt_table));
    com_write_str("acpi ok");
}
acpi *acpi::the()
{
    return &main_acpi;
}
