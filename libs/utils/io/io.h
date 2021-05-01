#ifndef BUFFER_H
#define BUFFER_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/container/container.h>
#include <utils/math.h>
#include <utils/type_traits.h>
namespace utils
{
    class reader
    {
    public:
        virtual size_t read(void *target, size_t count) = 0;
        template <typename T>
        T fetch()
        {
            T res;
            read(&res, sizeof(T));
            return res;
        }
        template <typename T>
        bool fetch(T &value)
        {
            return read(&value, sizeof(T)) == sizeof(T);
        }
    };

    class seeker
    {
    public:
        /* for whence value see stdio.h
         * note: a seekable must implement all seek value
         * it must return cur(), the current position
         */

        virtual size_t seek(long target_index, size_t whence = SEEK_SET) = 0;
        virtual size_t cur() const = 0;

        /* size() can be replaced if the virtual class can just do
         * size_t size() { return file_size;};
         * for exemple
         */
        virtual size_t size()
        {
            size_t current_offset = cur();

            // go to the end and go back to the previous position
            seek(0, SEEK_END);
            size_t res = cur();
            seek(current_offset, SEEK_SET);

            return res;
        }
    };

    class writer
    {
    public:
        virtual size_t write(const void *target, size_t count) = 0;

        template <typename T>
        bool put_back(const T &value)
        {
            return write(&value, sizeof(T)) == sizeof(T);
        }
    };

    template <typename T>
    concept read_only = is_base_of<reader, T>::value; // read only

    template <typename T>
    concept write_only = is_base_of<writer, T>::value; // write only

    template <typename T>
    concept duplex = is_base_of<reader, T>::value &&is_base_of<writer, T>::value; // read write

    template <typename T>
    concept seekable_readable = is_base_of<reader, T>::value &&is_base_of<seeker, T>::value; // seekable and readable

    template <typename T>
    concept seekable_writable = is_base_of<writer, T>::value &&is_base_of<seeker, T>::value; // seekable and writable

    template <typename T>
    concept seekable_duplex = is_base_of<writer, T>::value &&is_base_of<seeker, T>::value &&is_base_of<reader, T>::value; // seekable, writable & readable

    /*
     * basic_seeker_implementation is just a basic seeker implementation
     * with only getting the size, it is just here to avoid code repetition
     */
    class basic_seeker_implementation : public seeker
    {
    protected:
        long _scur;
        size_t _send; // must be set to the size/end by the implementation
    public:
        size_t cur() const final { return _scur; };
        size_t seek(long target_index, size_t whence = SEEK_SET) final;
    };

} // namespace utils

#endif // BUFFER_H
