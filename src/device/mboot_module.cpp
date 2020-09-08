#include <arch/mem/liballoc.h>
#include <com.h>
#include <device/mboot_module.h>
mboot_module main_boot_module_list;
mboot_module::mboot_module()
{
}

void mboot_module::init(stivale_struct *main_struct)
{
    stistruct = main_struct;
    printf("mboot module count %x \n", main_struct->module_count);
    modules = (stivale_module **)malloc(main_struct->module_count + 2); // 2 modules 1 for security and 1 for 0
    for (int i = 0; i < main_struct->module_count + 2; i++)
    {
        modules[i] = 0;
    }
    uint64_t at = main_struct->modules;
    for (int i = 0; i < main_struct->module_count; i++)
    {
        modules[i] = reinterpret_cast<stivale_module *>(at);
        at = modules[i]->next;
        printf("detected module %s, at %x, end %x", modules[i]->string, modules[i]->begin, modules[i]->end);
    }
}

mboot_module *mboot_module::the()
{
    return &main_boot_module_list;
}
