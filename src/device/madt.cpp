#include <device/madt.h>
#include <device/acpi.h>
madt main_madt;
madt::madt()
{

}


void madt::init(){
    madt_address = acpi::the()->find_entry("APIC");
}
madt* madt::the(){
    return &main_madt;
}
