#ifndef MEMORY_H
#define MEMORY_H
#include <assert.h>
#include <stddef.h>
#include <string.h>
namespace utils
{
    /* NOTE: this is for replacing c function argument
     *  void f(void* data, size_t size);
     * with:
     *  void f(utils::memory mem);
     *
     * it's like a unique_ptr, but is different in some ways
     *
     * NOTE2: the memory_buffer in buffer.h will be removed as it is bad
     */
    class memory
    {
    protected:
        size_t _size;
        void *_data;

        template <typename V, size_t type_size>
        void set_impl(const V value)
        {
            assert(type_size <= _size);
            V *d = reinterpret_cast<V *>(data());
            for (size_t i = 0; i < _size / type_size; i++)
            {
                d[i] = value;
            }
        }

        template <typename V>
        constexpr void set_impl(const V value)
        {
            V *d = reinterpret_cast<V *>(data());
            for (size_t i = 0; i < _size; i++)
            {
                d[i] = value;
            }
        }

    public:
        memory() : _size(0), _data(nullptr){}; // for creation use static function

        memory(const memory &) = delete; // no copy, use memory::copy

        memory(memory &&mem)
        {
            _size = mem._size;
            _data = mem.release();
        };
        template <typename V>
        void set(const V value)
        {
            if constexpr (sizeof(V) == sizeof(uint8_t))
            {
                set_impl<V>(value); // remove some check for type higher than a byte
            }
            else
            {
                set_impl<V, sizeof(V)>(value);
            }
        }
        void set(const memory &from);
        void set(const void *data, size_t size);

        bool compare(const memory &target) const;
        bool operator==(const memory &target) const { return compare(target); };
        bool operator!=(const memory &target) const { return !compare(target); };

        void *data() { return _data; }
        const void *data() const { return _data; }

        uint8_t get_byte(size_t idx) const
        {
            assert(_data);
            assert(idx < _size);
            return ((uint8_t *)_data)[idx];
        };
        void set_byte(size_t idx, uint8_t value)
        {
            assert(_data);
            assert(idx < _size);
            ((uint8_t *)_data)[idx] = value;
        };

        uint8_t &operator[](size_t index)
        {
            return ((uint8_t *)_data)[index];
        }
        const uint8_t operator[](size_t index) const
        {
            return ((const uint8_t *)_data)[index];
        }

        size_t size() const { return _size; };
        operator const void *() const { return data(); };
        operator void *() { return data(); };

        operator bool() const { return _data != nullptr; };

        void *release();

        void destroy();

        void give_ownership(void *data, size_t size);

        static memory create(size_t size);
        // now the memory object has the data AND manage to free it
        static memory create_and_give_ownership(void *data, size_t size);
        static memory copy(const memory &from);
        static memory copy(const void *data, size_t size);

        ~memory() { destroy(); };
    };
} // namespace utils

#endif // MEMORY_H
