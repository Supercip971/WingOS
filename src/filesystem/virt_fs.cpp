#include <filesystem/virt_fs.h>

virt_fs::virt_fs()
{
    fs_module_addr = 0;
    fs_module_end = 0;
}

global_fs *main_global_fs;
global_fs *global_fs::the()
{
    return main_global_fs;
}
