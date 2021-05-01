
#include <com.h>
#include <device/mboot_module.h>
#include <kernel.h>
#include <logging.h>
#include <utility.h>
#include <utils/memory/liballoc.h>
mboot_module main_boot_module_list;
mboot_module::mboot_module()
{
}

void mboot_module::init(stivale_struct *main_struct)
{
    stistruct = main_struct;
    log("mboot", LOG_DEBUG) << "loading mboot mudules";
    log("mboot", LOG_INFO) << "mboot module count" << main_struct->module_count;

    for (uint64_t i = 0; i < main_struct->module_count + 2; i++)
    {
        modules[i] = 0;
    }

    uintptr_t at = main_struct->modules;

    for (uint64_t i = 0; i < main_struct->module_count; i++)
    {
        modules[i] = reinterpret_cast<stivale_module *>(at);
        at = modules[i]->next;
        log("mboot", LOG_INFO) << "detected module" << modules[i]->string << "at" << modules[i]->begin << "end" << modules[i]->end;
    }
}

stivale_module *mboot_module::get_fs_module()
{
    for (uint64_t i = 0; modules[i] != 0; i++)
    {
        if (strncmp("ramdisk", modules[i]->string, 7) == 0)
        {
            log("mboot", LOG_INFO) << "detected ram disk module at" << modules[i]->begin;

            return modules[i];
        }
    }

    log("mboot", LOG_ERROR) << "no detected ram disk module";
    return nullptr;
}

mboot_module *mboot_module::the()
{
    return &main_boot_module_list;
}
