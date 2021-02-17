#include "interpret_command.h"
#include <kern/file.h>
#include <stdio.h>
#include <string.h>
#include <utils/wstring.h>
int interpret_command(char *v)
{
    if (strcmp(v, "exit") == 1)
    {
        return INTERPRET_QUIT;
    }
    else
    {
        char *target_str = new char[255];
        for (size_t i = 0; i < strlen(v); i++)
        {
            if (v[i] == ' ')
            {
                target_str[i] = 0;
                break;
            }
            target_str[i] = v[i];
        }
        printf("running: %s \n", target_str);
        sys::file target = sys::file(target_str);
        if (target.is_openned())
        {

            target.close();
            delete[] target_str;
            return 0;
        }
        else
        {
        }
        delete[] target_str;
    }

    printf("error: command %s not available \n", v);
    return -1; // un handled command
}
