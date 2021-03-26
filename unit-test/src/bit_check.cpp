#include "bit_check.h"
#include <utils/bit.h>
int get_bit_check()
{
    uint8_t val1 = 0b01010101;

    for (int i = 0; i < 8; i++)
    {
        if (utils::get_bit(val1, i))
        {
            if ((i % 2))
            {
                return i + 1;
            }
        }
        else
        {
            if (!(i % 2))
            {
                return -i - 1;
            }
        }
    }
    return 0;
}
int set_bit_check()
{

    uint8_t val1 = 0b01010101;

    for (int i = 0; i < 8; i++)
    {
        if (utils::get_bit(val1, i))
        {
            utils::set_bit(val1, i, 0);
        }
        else
        {
            utils::set_bit(val1, i, 1);
        }
    }
    for (int i = 0; i < 8; i++)
    {
        if (!utils::get_bit(val1, i))
        {
            if ((i % 2))
            {
                return i + 1;
            }
        }
        else
        {
            if (!(i % 2))
            {
                return -i - 1;
            }
        }
    }
    return 0;
}
