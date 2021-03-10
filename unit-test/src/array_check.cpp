#include "array_check.h"
#include <utils/warray.h>
int array_creation_check()
{
    utils::array<int, 5> test;

    if (test.size() != 5)
    {
        return -2;
    }

    if (test.raw() == nullptr)
    { // this case should not exist
        return -3;
    }
    return 0;
}
int array_access_check()
{
    utils::array<size_t, 1024> test;
    if (test.size() != 1024)
    {
        return -2;
    }

    for (size_t i = 0; i < test.size(); i++)
    {
        test[i] = i;
    }

    for (size_t i = 0; i < test.size(); i++)
    {
        if (test[i] != i)
        {
            return i - 4096;
        };
    }

    return 0;
}
int array_fill_check()
{
    utils::array<int, 1024> test;
    test.fill(16); // why 16 ? i don't know !

    for (size_t i = 0; i < test.size(); i++)
    {
        if (test[i] != 16)
        {
            return i - 4096;
        };
    }
    return 0;
}
