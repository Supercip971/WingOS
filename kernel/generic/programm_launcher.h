#pragma once

#include <elf_gnu_structure.h>
#include <filesystem/echfs.h> // to do add a global file system

uint64_t launch_programm(const char *path, file_system *file_sys, int argc, const char **argv);
