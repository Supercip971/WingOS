#pragma once

#include <sys/types.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include <stddef.h>
    char *getcwd(char *buf, size_t size);

    int chdir(const char *path);

    void _exit(int status);

    pid_t fork();

#ifdef __cplusplus
}
#endif
