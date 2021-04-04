#include "memory_io.h"

namespace utils
{

    void memory_io::update_size(size_t new_size)
    {
        if (new_size >= _data_size)
        {
            _data_size = new_size * 2;
            if (_data == nullptr)
            {
                _data = malloc(_data_size);
            }
            else
            {
                _data = realloc(_data, _data_size);
            }
        }
        _size = new_size;
        _send = _size;
    }

    memory_io::memory_io(size_t preallocated_size) : memory_io()
    {
        update_size(preallocated_size);
    }

    size_t memory_io::read(void *target, size_t count)
    {
        size_t fcount = utils::min(count, _size - cur());
        memcpy(target, (uint8_t *)_data + cur(), fcount);
        return fcount;
    }
    size_t memory_io::read(void *target, size_t count) const
    {
        size_t fcount = utils::min(count, _size - cur());
        memcpy(target, (uint8_t *)_data + cur(), fcount);
        return fcount;
    }
    size_t memory_io::write(const void *target, size_t count)
    {
        size_t fcount = utils::min(count, _size - cur());
        memcpy((uint8_t *)_data + cur(), target, fcount);
        return fcount;
    }

    void memory_io::resize(size_t new_size)
    {
        update_size(new_size);
    }
    memory_io::memory_io(const memory_io &from) : memory_io()
    {
        if (!from._data)
        {
            destroy();
            return;
        }
        update_size(from._size + 2);
        seek(0, SEEK_SET);
        write(from._data, from._size);
        seek(from.cur(), SEEK_SET); // set same seek pos as from
    }
    memory_io::memory_io(const memory_io &&from) : memory_io()
    {

        if (!from._data)
        {
            destroy();
            return;
        }
        update_size(from._size + 2);
        seek(0, SEEK_SET);
        write(from._data, from._size);
        seek(from.cur(), SEEK_SET); // set same seek pos as from
        from.~memory_io();
    }

    memory_io &memory_io::operator=(const memory_io &&from)
    {

        if (!from._data)
        {
            destroy();
            return *this;
        }
        update_size(from._size);
        seek(0, SEEK_SET);
        write(from._data, from._size);
        seek(from.cur(), SEEK_SET); // set same seek pos as from
        from.~memory_io();
        return *this;
    }
    memory_io &memory_io::operator=(const memory_io &from)
    {
        if (!from._data)
        {
            destroy();
            return *this;
        }
        update_size(from._size);
        seek(0, SEEK_SET);
        write(from._data, from._size);
        seek(from.cur(), SEEK_SET); // set same seek pos as from
        return *this;
    }
} // namespace utils
