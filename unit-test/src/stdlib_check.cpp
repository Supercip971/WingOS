#include <stdio.h>
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

int atoi_check()
{
    int val = atoi("16");

    if (val != 16)
    {
        return 16;
    }
    val = atoi("32eeee");

    if (val != 32)
    {
        return 32;
    }
    val = atoi("1234567890eeee");
    if (val != 1234567890)
    {
        return 12;
    }
    return 0;
}
