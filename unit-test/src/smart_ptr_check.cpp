#include "smart_ptr_check.h"
#include <utils/smart_ptr.h>
#include <utils/type_traits.h>

int unique_ptr_create_destroy_check()
{

    {

        utils::unique_ptr<int> null_unique(nullptr);
        if (null_unique)
        {
            return -1;
        }
        null_unique.~unique_ptr();
        if (null_unique)
        {
            return -2;
        }
    }

    {
        utils::unique_ptr<int> nonnull_unique(new int[2]);

        if (!nonnull_unique)
        {
            return -3;
        }
        nonnull_unique.~unique_ptr();

        if (nonnull_unique)
        {
            return -4;
        }
    }
    return 0;
}
int unique_ptr_raw_check()
{

    int *data = new int[10];
    utils::unique_ptr<int> nonnull_unique(data);
    if (!nonnull_unique)
    {
        return -1;
    }
    if (nonnull_unique.get_raw() != data)
    {
        return -2;
    }

    return 0;
}

int unique_ptr_operator_check()
{
    int *data = new int[10];

    for (int i = 0; i < 10; i++)
    {
        data[i] = i;
    }
    utils::unique_ptr<int> nonnull_unique(data);

    if (!nonnull_unique)
    {
        return -1;
    }

    if (nonnull_unique.get() != 0)
    {
        return -2;
    }

    for (int i = 0; i < 10; i++)
    {
        if (nonnull_unique[i] != i)
        {
            return 1024 + i;
        }
    }

    return 0;
}
int unique_ptr_reset_check()
{

    int *data = new int[10];
    int *data2 = new int[10];

    data[5] = 0;
    data2[5] = 10;

    utils::unique_ptr<int> nonnull_unique(data);

    if (nonnull_unique.get_raw() != data)
    {
        return -1;
    }

    nonnull_unique.reset(data2);

    if (nonnull_unique[5] == 0)
    {
        return -2;
    }
    if (nonnull_unique.get_raw() != data2)
    {
        return -3;
    }

    if (!nonnull_unique)
    {
        return -4;
    }

    return 0;
}
int unique_ptr_release_check()
{

    int *data = new int[10];
    utils::unique_ptr<int> nonnull_unique(data);
    if (nonnull_unique.get_raw() != data)
    {
        return -1;
    }

    int *res = nonnull_unique.release();

    if (nonnull_unique)
    {
        return -2;
    }

    if (res != data)
    {
        return -3;
    }

    delete[] res;

    return 0;
}

int make_unique_check()
{

    auto nonnull_unique = (utils::make_unique<int>(10));
    if (!nonnull_unique)
    {
        return -1;
    }
    if (nonnull_unique.get() != 10)
    {
        return -2;
    }

    return 0;
}
