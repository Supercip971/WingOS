#pragma once

#include <arch/elf_gnu_structure.h>
#include <filesystem/echfs.h> // to do add a global file system

void launch_programm(const char *path, echfs *file_sys);
