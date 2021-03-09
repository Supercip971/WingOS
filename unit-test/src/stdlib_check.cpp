#include <stdlib.h>

int abs_check()
{
    if (abs(-32) != 32)
    {
        return -1;
    }
    if (abs(39327) != 39327)
    {
        return -2;
    }
    return 0;
}