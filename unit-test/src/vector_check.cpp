#include "vector_check.h"

#include <stdio.h>

#include <utils/wvector.h>

int wvector_create_check()
{
    utils::vector<uint8_t> vec = utils::vector<uint8_t>();
    if (vec)
    {
        return 1;
    }
    else if (vec.size() != 0)
    {
        return 2;
    }
    return 0;
}
int wvector_push_back_check(){

    utils::vector<uint8_t> vec = utils::vector<uint8_t>();
int wvector_push_back_check()
{

    utils::vector<uint8_t> vec = utils::vector<uint8_t>();
    for (int i = 0; i < 2048; i++)
    {
        vec.push_back(0);
    }
    if (!vec)
    {
        return -1;
    }
    if (vec.size() != 2048)
    {
        return vec.size();
    }
    return 0;
}
int wvector_get_check()
{
    utils::vector<int> vec = utils::vector<int>();
    for (int i = 0; i < 2048; i++)
    {
        vec.push_back(i);
    }
    if (!vec)
    {
        return -1;
    }
    if (vec.size() != 2048)
    {
        return vec.size();
    }
    for (int i = 0; i < 2048; i++)
    {
        if (vec.get(i) != vec[i])
        {
            return i + 20000;
        }
        if (vec.get(i) != i)
        {
            printf("invalid i %i = %i \n", i, vec[i]);
            return i + 10000;
        }
    }

    return 0;
}
int wvector_remove_check()
{
    utils::vector<int> vec = utils::vector<int>();
    for (int i = 0; i < 4096; i++)
    {
        vec.push_back(i);
    }
    if (!vec)
    {
        return -1;
    }
    int off = 0;
    for (int i = 0; i < 4096; i++)
    {
        if ((i % 2) != 0)
        {
            vec.remove(i + off);
            off -= 1;
        }
    }
    if (!vec)
    {
        return -2;
    }
    if (vec.size() != 2048)
    {
        return vec.size();
    }
    for (int i = 0; i < 2048; i++)
    {
        if (vec.get(i) != vec[i])
        {
            return i + 20000;
        }
        if (vec.get(i) != i * 2)
        {
            printf("invalid i %i = %i \n", i, vec[i]);
            return i + 10000;
        }
    }
    return 0;
}
int wvector_clear_check()
{
    utils::vector<int> vec = utils::vector<int>();
    for (int i = 0; i < 4096; i++)
    {
        vec.push_back(i);
    }
    if (!vec)
    {
        return -1;
    }
    vec.clear();

    if (vec)
    {
        return -2;
    }
    if (vec.size() != 0)
    {
        return vec.size();
    }
    return 0;
}
