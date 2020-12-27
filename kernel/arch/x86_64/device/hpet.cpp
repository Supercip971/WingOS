#include <com.h>
#include <device/acpi.h>
#include <device/apic.h>
#include <device/hpet.h>
#include <logging.h>
hpet main_hpet;
hpet::hpet()
{
}
void hpet::init_hpet()
{
    // why setting up log for hpet if we doesn't use it ?
    log("hpet", LOG_DEBUG) << "loading hpet";
    main_hpet_entry = (entry_hpet *)acpi::the()->find_entry("HPET");
    hpet_main_structure = (main_hpet_struct *)main_hpet_entry->address.address;
    uint64_t temporary_check = 0;
    temporary_check = hpet_main_structure->general_capabilities;

    uint64_t clock_period = temporary_check >> 32;
    uint64_t freq = 1000000000000000 / clock_period;

    log("hpet", LOG_INFO) << "hpet frequency" << freq;

    printf("HPET frequency = %x \n", freq);

    printf("HPET checking for validity ..\n");

    if (!(hpet_main_structure->general_capabilities & GENERAL_LEGACY_REPLACEMENT))
    {
        printf("ERROR :HPET is not legacy replacement capable");
        return;
    }

    hpet_individual_timer *first_timer = &hpet_main_structure->timers[0];

    if (!(first_timer->cfg_what_can_i_do & H_PERIODIC_MODE_SUPPORT))
    {
        printf("ERROR : HPER the first timer doesn't support periodic mode :(");
    }

    first_timer->cfg_what_can_i_do |= H_ENABLE_INTERRUPT;
    first_timer->cfg_what_can_i_do |= H_ENABLE_PERIODIC_MODE;
    first_timer->cfg_what_can_i_do |= H_SET_PERIODIC_ACCUMULATOR;
    first_timer->comp_value = freq / 1000 /* this is the target frequency */;

    hpet_main_structure->general_configuration |= 0b01;

    printf("turning on hpet");

    hpet_main_structure->general_configuration |= 0b10; // enabling legacy replacement mode

    hpet_main_structure->main_counter_value = 0;

    apic::the()->set_redirect_irq(0, 0, 1);
}

hpet *hpet::the()
{
    return &main_hpet;
}
