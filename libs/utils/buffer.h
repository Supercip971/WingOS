#ifndef BUFFER_H
#define BUFFER_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/math.h>
namespace utils
{
    class buffer
    {
    public:
        buffer(){};
        ~buffer()
        {
            if (can_be_destroyied())
            {
                destroy();
            }
        };

        bool can_be_destroyied() const { return false; }; // a buffer must not be destroyable when it is const
        virtual bool can_be_destroyied() { return false; };

        virtual void destroy(){};

        virtual bool is_seekable() const { return false; };
        virtual size_t cur() const = 0;
        virtual size_t seek(int target_index, int whence) = 0;

        virtual bool is_readable() const { return false; };
        virtual bool is_readable() { return false; };
        virtual size_t read(void *target, size_t count) = 0;
        virtual size_t read(void *target, size_t count, size_t at) const = 0;

        bool is_writable() const { return false; }; // a buffer must not be writable when it is const
        virtual bool is_writable() { return false; };
        virtual size_t write(const void *target, size_t count) = 0;
        virtual size_t write(const void *target, size_t count, size_t at) = 0;

        bool can_be_resized() const { return false; }; // a buffer must not be resizable when it is const
        virtual bool can_be_resized() { return false; };

        virtual void resize(size_t new_count) = 0;

        virtual bool can_get_raw() const { return true; };
        virtual bool can_get_raw() { return true; };
        virtual const uint8_t *raw() const = 0;
        virtual uint8_t *raw() = 0;

        virtual bool can_get_size() const { return false; };
        virtual size_t get_size() const = 0;

        virtual bool can_get_size() { return false; };
        virtual size_t get_size() = 0;

        template <class V>
        V read_type()
        {
            if (is_readable())
            {
                V value;
                read(&value, sizeof(value));
                return value;
            }
            else
            {
                return V();
            }
        }
        operator const uint8_t *() const
        {
            if (can_get_raw())
            {
                return raw();
            }
            return nullptr;
        }
        operator uint8_t *()
        {
            if (can_get_raw())
            {
                return raw();
            }
            return nullptr;
        }
        // const version
    };

    inline bool copy(buffer *to, const buffer *from)
    {
        to->resize(from->get_size());
        uint8_t *copy = new uint8_t[from->get_size()];
        from->read(copy, from->get_size(), 0);
        to->write(copy, from->get_size());
        delete[] copy;
        return true;
    }
    inline bool copy(buffer *to, const buffer *from, size_t length)
    {
        size_t copy_length = min(length, from->get_size());
        to->resize(copy_length);
        uint8_t *copy = new uint8_t[from->get_size()];
        from->read(copy, from->get_size(), 0);
        to->write(copy, from->get_size());
        delete[] copy;
        return true;
    }

    template <class T>
    class memory_buffer : public buffer
    {
    protected:
        T *data;

        size_t cursor;
        size_t allocated_size;
        size_t current_data_size;

    public:
        static_assert(sizeof(T) != 0, "size of memory buffer entry must has a size > 0");
        memory_buffer(size_t size)
        {
            data = (T *)malloc(size);
            current_data_size = size * sizeof(T);
            allocated_size = size * sizeof(T);
        }
        memory_buffer(const T *from_data, size_t size)
        {
            data = (T *)malloc(size);
            current_data_size = size;
            allocated_size = size;
            memcpy(data, from_data, size);
        }

        bool can_be_destroyied() const { return true; };
        virtual void destroy()
        {
            free(data);
        }

        virtual bool is_seekable() const { return true; };
        virtual size_t cur() const { return cursor; };
        virtual size_t seek(int target_index, int whence = SEEK_SET)
        {
            if (whence == SEEK_SET)
            {
                cursor = target_index;
            }
            else if (whence == SEEK_END)
            {
                cursor = current_data_size;
            }
            else if (whence == SEEK_CUR)
            {
                cursor += target_index;
            }
            else
            {
                return 0;
            }
            return cur();
        };

        virtual bool is_readable() const { return true; };
        virtual bool is_readable() { return true; };
        virtual size_t read(void *target, size_t size)
        {
            size_t v = size;
            if (cursor > v)
            {
                return 0;
            }
            else if (cursor + size > current_data_size)
            {
                v = current_data_size - cursor;
            }
            memcpy(target, data, v);

            cursor += v;
            return v;
        };

        virtual size_t read(void *target, size_t size, size_t at) const
        {
            size_t v = size;
            if (at > v)
            {
                return 0;
            }
            else if (at + size > current_data_size)
            {
                v = current_data_size - at;
            }
            memcpy(target, data, v);

            return v;
        };

        virtual size_t read_value(T *target, size_t size)
        {
            size_t v = size;
            if (cursor > v)
            {
                return 0;
            }
            else if (cursor + size > current_data_size)
            {
                v = current_data_size - cursor;
            }
            memcpy((uint8_t *)target, (uint8_t *)data, v);

            cursor += v;
            return v;
        };

        virtual bool is_writable() { return true; };
        virtual size_t write(const void *target, size_t size)
        {

            size_t v = size;
            if (cursor > v)
            {
                return 0;
            }
            else if (cursor + size > current_data_size)
            {
                v = current_data_size - cursor;
            }
            memcpy((uint8_t *)data, (uint8_t *)target, v);

            cursor += v;
            return v;
        }
        virtual size_t write(const void *target, size_t size, size_t at)
        {

            size_t v = size;
            if (at > v)
            {
                return 0;
            }
            else if (at + size > current_data_size)
            {
                v = current_data_size - at;
            }
            memcpy((uint8_t *)data, (uint8_t *)target, v);

            return v;
        }
        virtual size_t write_value(const T *target, size_t size)
        {

            size_t v = size;
            if (cursor > v)
            {
                return 0;
            }
            else if (cursor + size > current_data_size)
            {
                v = current_data_size - cursor;
            }
            memcpy((uint8_t *)data, (uint8_t *)target, v);

            cursor += v;
            return v;
        }

        virtual bool can_be_resized() { return true; };
        void resize(size_t new_size)
        {
            current_data_size = new_size;
            if (current_data_size > allocated_size)
            {
                allocated_size = current_data_size + 16;
                data = (T *)realloc(data, allocated_size);
            }
        }

        virtual bool can_get_raw() { return true; };
        virtual uint8_t *raw()
        {
            return (uint8_t *)data;
        };
        const T *raw_value() const
        {
            return data;
        };
        T *raw_value()
        {
            return data;
        };

        virtual bool can_get_raw() const { return true; };
        virtual const uint8_t *raw() const
        {
            return (uint8_t *)data;
        };
        operator uint8_t *()
        {
            return raw();
        }

        virtual bool can_get_size() const { return true; };
        virtual bool can_get_size() { return true; };
        virtual size_t get_size() const { return current_data_size; };
        virtual size_t get_size() { return current_data_size; };
    };
    using raw_memory_buffer = memory_buffer<uint8_t>;

} // namespace utils

#endif // BUFFER_H
