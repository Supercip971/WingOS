#ifndef APP_LAUNCHER_H
#define APP_LAUNCHER_H

struct programm_exec_info
{
    const char *path;
    const char *name;
    int argc;
    char **argv;

    char *env; // not supported
};

#endif // APP_LAUNCHER_H
