#ifndef WSTRING_H
#define WSTRING_H
#include <stdio.h>
#include <string.h>
#include <utils/buffer.h>
namespace utils
{

    class string;
    template <typename V>
    string to_str(V value);
    template <>
    string to_str<const char *>(const char *value);

    class string : public memory_buffer<char>
    {
    protected:
        char null_return = 0;
        size_t string_length;

    public:
        string() : memory_buffer<char>(16)
        {
            string_length = 0;
        }
        string(const char *value) : memory_buffer<char>(strlen(value) + 16)
        {
            this->seek(0);
            this->write_value(value, strlen(value) + 1);
            string_length = strlen(value);
        }
        string(const char value) : memory_buffer<char>(16)
        {
            this->seek(0);
            this->write_value(&value, 1);
            this->seek(1);
            char null_str = 0;
            this->write_value(&null_str, 1);
            string_length = 1;
        }
        string(const char *value, size_t length) : memory_buffer<char>(length + 16)
        {
            this->seek(0);
            this->write_value(value, length);
            string_length = length;
        }
        string(const string &value) : memory_buffer<char>((value.length()) + 16)
        {
            this->seek(0);
            this->write_value(value.raw_value(), value.length() + 1);
            string_length = value.length();
        }
        string(string &value) : memory_buffer<char>((value.length()) + 16)
        {
            this->seek(0);
            this->write_value(value.raw_value(), value.length() + 1);
            string_length = value.length();
        }
        string(const buffer *value) : memory_buffer<char>((value->get_size()) + 16)
        {
            this->seek(0);
            copy(this, value);

            string_length = strlen((char *)value->raw());
        }
        ~string()
        {
            destroy();
        }
        template <typename V>
        string(V value) : string(to_str<V>(value))
        {
        }

        char &operator[](size_t v)
        {
            return *((this->raw_value()) + v);
        }
        char operator[](size_t v) const
        {
            return *((this->raw_value()) + v);
        }
        char &get(size_t v)
        {
            if (v > length())
            {
                return null_return;
            }
            return *((this->raw_value()) + v);
        }
        char get(size_t v) const
        {
            if (v > length())
            {
                return null_return;
            }
            return *((this->raw_value()) + v);
        }

        const char *c_str() const
        {
            return raw_value();
        }

        void append(const string value)
        {
            size_t added_length = value.length();
            resize(string_length + added_length + 1);
            seek(string_length);
            write(value.raw(), value.length() + 1);
            string_length += added_length;
        }
        string &operator+=(const string v)
        {
            append(v);
            return *this;
        }
        size_t length() const { return string_length; };
    };

} // namespace utils

#endif // STRING_H
