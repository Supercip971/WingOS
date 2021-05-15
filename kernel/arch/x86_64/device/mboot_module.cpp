
#include <arch.h>
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

void mboot_module::init(stivale2_struct *main_struct)
{
    log("mboot", LOG_ERROR, "todo: reimplement module (for the moment the kernel don't use module)");
}

stivale2_module *mboot_module::get_fs_module()
{
    for (uint64_t i = 0; modules[i] != 0; i++)
    {
        if (strncmp("ramdisk", modules[i]->string, 7) == 0)
        {
            log("mboot", LOG_INFO, "detected ram disk module at: {}", modules[i]->begin);

            return modules[i];
        }
    }

    log("mboot", LOG_ERROR, "no detected ram disk module");
    return nullptr;
}

mboot_module *mboot_module::the()
{
    return &main_boot_module_list;
}
