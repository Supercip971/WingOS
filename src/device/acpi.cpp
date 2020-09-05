#include <arch/mem/virtual.h>
#include <com.h>
#include <device/acpi.h>
#include <device/apic.h>
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
            printf("acpi: Found RSDP at %x \n", i);
            return (void *)i;
        }
    }

    return nullptr;
}
acpi::acpi()
{
}

void *acpi::find_entry(const char *entry_name)
{
    RSDT *rsdt = (RSDT *)descriptor->firstPart.RSDT_address;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        printf("searching ... \n");

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            printf("skipping entry 0 \n");
            continue;
        }
        RSDTHeader *h = (RSDTHeader *)(rsdt->PointerToOtherSDT[i]);

        if (!strncmp(h->Signature, entry_name, 4))
            return (void *)h;
    }

    // Not found
    return nullptr;
}
void *findFACP(void *RootSDT)
{
    RSDT *rsdt = (RSDT *)RootSDT;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        printf("searching ... \n");

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            printf("skipping entry 0 \n");
            continue;
        }
        RSDTHeader *h = (RSDTHeader *)(rsdt->PointerToOtherSDT[i]);

        if (!strncmp(h->Signature, "FACP", 4))
            return (void *)h;
    }

    // No FACP found
    return nullptr;
}
void acpi::init(uint64_t rsdp)
{
    printf("acpi 1 \n");
    descriptor = (RSDPDescriptor20 *)((((uint64_t)get_rsdp() /* get with the offset as stivale doesn't give me the good address so fu** you stivale */)));
    printf("acpi 2 \n");
    rsdt_table = (RSDT *)(descriptor->firstPart.RSDT_address);
    printf("acpi 3 \n");
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
        printf("searching ... \n");

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            printf("skipping entry 0 \n");
            continue;
        }
        RSDTHeader *h = (RSDTHeader *)(rsdt->PointerToOtherSDT[i]);

        printf(h->Signature);
        printf(" \n");
    }
    printf("FACP : %x \n", (uint64_t)findFACP(rsdt_table));
    printf("acpi ok \n");
}
acpi *acpi::the()
{
    return &main_acpi;
}
