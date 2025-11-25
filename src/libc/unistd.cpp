#include "unistd.h"
#include <iol/iol.h>
extern "C" char *getcwd(
    char *buf, size_t size)
{
    
    char * b = iol_get_cwd();
    if (buf == NULL)
    {
        return NULL;
    }

    size_t len = 0;
    while (b[len] != '\0' && len < size - 1)
    {
        buf[len] = b[len];
        len++;
    }

    return buf;

}