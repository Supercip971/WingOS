#ifndef MEMORY_IO_H
#define MEMORY_IO_H
#include <utils/io/io.h>
#include <utils/memory/smart_ptr.h>
namespace utils
{
    class memory_io : public reader,
                      public writer,
                      public basic_seeker_implementation // seeker
    {
        void *_data;
        size_t _size;
        size_t _data_size;

        void update_size(size_t new_size);
        void destroy()
        {
            if (_data)
            {
                _size = 0;
                _data_size = 0;
                free(_data);
            }
            _data = nullptr;
        }

    public:
        memory_io() : _data(nullptr), _size(0), _data_size(0)
        {
            _send = (0);
            _scur = (0);
        };
        memory_io(size_t preallocated_size);
        memory_io(const memory_io &from);  // copy
        memory_io(const memory_io &&from); // move

        size_t read(void *target, size_t count) final;
        size_t read(void *target, size_t count) const;
        size_t write(const void *target, size_t count) final;

        size_t size() final { return _size; };

        void resize(size_t new_size);

        void *release()
        {
            void *saved_data = _data;
            _data = nullptr;
            _size = 0;
            _data_size = 0;
            return saved_data;
        };

        void *data() { return _data; };
        const void *data() const { return _data; };

        virtual ~memory_io()
        {
            destroy();
        }

        uint8_t &operator[](size_t val)
        {
#ifdef DEBUG
            if (val > _size)
            {
                printf("oob access of memory io 0x%x > 0x%x \n", val, _size);
            }
#endif
            return ((uint8_t *)_data)[val];
        }
        uint8_t operator[](size_t val) const
        {
#ifdef DEBUG
            if (val > _size)
            {
                printf("oob access of memory io 0x%x > 0x%x \n", val, _size);
            }
#endif
            return ((uint8_t *)_data)[val];
        }

        memory_io &operator=(const memory_io &&from);
        memory_io &operator=(const memory_io &from);
    };
} // namespace utils

#endif // MEMORY_IO_H
