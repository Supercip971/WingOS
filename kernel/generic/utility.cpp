#include <stdlib.h>
#include <utility.h>
#include <utils/memory/liballoc.h>

void kitoaT(char *buf, int base, size_t d)
{
    char *p = buf;
    char *p1, *p2;
    size_t ud = d;
    size_t divisor = 10;

    if (base == 'd' && d < 0)
    {
        *p++ = '-';
        buf++;
        ud = -d;
        divisor = 10;
    }
    else if (base == 'x')
    {
        divisor = 16;
    }

    do
    {
        size_t remainder = ud % divisor;

        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);

    *p = 0;
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2)
    {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}
