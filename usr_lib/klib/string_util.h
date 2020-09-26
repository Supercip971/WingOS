#pragma once

namespace sys {

    template <class T>
    inline void int_to_string(char *buf, int base, T d)
    {
        char *p = buf;
        char *p1, *p2;
        T ud = d;
        T divisor = 10;

        if (base == 'd' && d < 0)
        {
            *p++ = '-';
            buf++;
            ud = -d;
        }
        else if (base == 'x')
            divisor = 16;

        do
        {
            T remainder = ud % divisor;

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
}
