#include "memory.h"
#include <utils/math.h>
#include <utils/type_traits.h>
namespace utils
{
    bool memory::compare(const memory &target) const
    {
        if (_size != target._size)
        {
            return false;
        }
        const char *one = reinterpret_cast<const char *>(data());
        const char *two = reinterpret_cast<const char *>(target.data());

        for (size_t i = 0; i < _size; i++)
        {
            if (one[i] != two[i])
            {
                return false;
            }
        }
        return true;
    }

    void *memory::release()
    {

        _size = 0;
        void *temp = _data;
        _data = nullptr;
        return temp;
    }

    void memory::destroy()
    {
        if (_data != nullptr)
        {
            free(_data);
            _data = nullptr;
            _size = 0;
        }
    }
    memory memory::create(size_t size)
    {
        memory target;
        target._data = malloc(size);
        target._size = size;

        return (target);
    }

    memory memory::create_and_give_ownership(void *data, size_t size)
    {

        memory target;
        target.give_ownership(data, size);
        return target;
    }
    memory memory::copy(const memory &from)
    {
        memory target;
        target._data = malloc(from._size);
        target._size = from._size;
        target.set(from);

        return (target);
    }

    memory memory::copy(const void *data, size_t size)
    {
        memory target;
        target._data = malloc(size);
        target._size = size;
        target.set(data, size);

        return (target);
    }

    void memory::set(const memory &from)
    {

        char *one = reinterpret_cast<char *>(data());
        const char *two = reinterpret_cast<const char *>(from.data());
        const size_t min_size = min(from._size, _size);
        for (size_t v = 0; v < min_size; v++)
        {
            one[v] = two[v];
        }
    }
    void memory::set(const void *data, size_t size)
    {

        char *one = reinterpret_cast<char *>(_data);
        const char *two = reinterpret_cast<const char *>(data);
        const size_t min_size = min(_size, size);
        for (size_t v = 0; v < min_size; v++)
        {
            one[v] = two[v];
        }
    }
    void memory::give_ownership(void *data, size_t size)
    {
        destroy();
        _data = data;
        _size = size;
    }

} // namespace utils
