#ifndef MBOOT_MODULE_H
#define MBOOT_MODULE_H

#include <stivale_struct.h>
class mboot_module
{
    stivale2_struct *stistruct;
    stivale2_module *modules[50]; // that's an array of pointer rip
public:
    mboot_module();
    void init(stivale2_struct *main_struct);
    stivale2_module *get_fs_module();
    static mboot_module *the();
};

#endif // MBOOT_MODULE_H
