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

    int pipe(int fds[2]);

    int close(int fd); // todo: see difference between fclose and close ?

    int dup2(int oldfd, int newfd);

    int execve(const char *path, char *const argv[],
               char *const envp[]);

    ssize_t read(int fd, void *buf, size_t len);

#ifdef __cplusplus
}
#endif
