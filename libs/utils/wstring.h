#ifndef WSTRING_H
#define WSTRING_H
#include <stdio.h>
#include <string.h>
#include <utils/io/memory_io.h>
namespace utils
{

    class string;

    template <typename V>
    string to_str(V value);

    template <>
    string to_str<const char *>(const char *value);

    class string : protected memory_io
    {

    protected:
        char null_return = 0;
        size_t string_length;

    public:
        string() : memory_io(16)
        {
            string_length = 0;
        }

        string(const char *value) : memory_io(strlen(value) + 16)
        {
            this->seek(0);
            this->write(value, strlen(value) + 1);
            string_length = strlen(value);
        }

        string(const char value) : memory_io(16)
        {
            this->seek(0);
            put_back<const char>(value);
            this->seek(1);
            put_back<const char>(0);
            string_length = 1;
        }

        string(const char *value, size_t length) : memory_io(length + 16)
        {
            this->seek(0);
            this->write(value, length);
            string_length = length;
        }

        string(const string &value) : memory_io((value.length()) + 16)
        {
            this->seek(0);
            this->write(value.data(), value.length() + 1);
            string_length = value.length();
        }

        string(string &value) : memory_io((value.length()) + 16)
        {
            this->seek(0);
            this->write(value.data(), value.length() + 1);
            string_length = value.length();
        }

        ~string()
        {
        }

        template <typename V>
        string(V value) : string(to_str<V>(value))
        {
        }

        size_t length() const { return string_length; };

        char &operator[](size_t v)
        {
            return *((char *)this->data() + v);
        }

        char operator[](size_t v) const
        {
            return *((char *)this->data() + v);
        }

        char &get(size_t v)
        {
            if (v > length())
            {
                return null_return;
            }
            return *((char *)this->data() + v);
        }

        char get(size_t v) const
        {
            if (v > length())
            {
                return null_return;
            }
            return *((char *)this->data() + v);
        }

        const char *c_str() const
        {
            return (const char *)data();
        }

        void append(const string &value)
        {
            size_t added_length = value.length();

            resize(string_length + added_length + 1);
            seek(string_length);
            write(value.data(), value.length() + 1);

            string_length += added_length;
        }
        string &operator+=(const string &v)
        {
            append(v);
            return *this;
        }
        bool operator==(const string &v) const;
        bool operator!=(const string &v) const {return !(*this == v);};
    };

} // namespace utils

#endif // STRING_H
