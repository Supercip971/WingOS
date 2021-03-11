#ifndef UNICODE_H
#define UNICODE_H
#include <stddef.h>
#include <stdint.h>
namespace utils
{
    typedef uint32_t unicode_codepoint;
    constexpr bool is_unicode_digit(const unicode_codepoint point)
    {
        if (point <= U'9' && point >= U'0')
        {
            return true;
        }
        return false;
    }

    constexpr bool is_unicode_alpha(const unicode_codepoint point)
    {
        if (point <= U'a' && point >= U'z')
        {
            return true;
        }
        else if (point <= U'A' && point >= U'Z')
        {
            return true;
        }
        return false;
    }

    constexpr bool is_unicode_alnum(const unicode_codepoint point)
    {
        return is_unicode_digit(point) || is_unicode_alpha(point);
    }

    constexpr int utf8_from_codepoint(const unicode_codepoint target, char *target_buffer)
    {
        if (target < 0x80)
        {
            target_buffer[0] = (char)target;
            target_buffer[1] = 0;

            return 1;
        }
        else if (target < 0x800)
        {
            target_buffer[0] = (char)(((target >> 6) & 0x1F) | 0xC0);
            target_buffer[1] = (char)(((target)&0x3F) | 0x80);
            target_buffer[2] = 0;

            return 2;
        }
        else if (target < 0x10000)
        {
            target_buffer[0] = (char)(((target >> 12) & 0x0F) | 0xE0);
            target_buffer[1] = (char)(((target >> 6) & 0x3F) | 0x80);
            target_buffer[2] = (char)(((target >> 0) & 0x3F) | 0x80);
            target_buffer[3] = 0;
            return 3;
        }
        else if (target < 0x110000)
        {
            target_buffer[0] = (char)(((target >> 18) & 0x07) | 0xF0);
            target_buffer[1] = (char)(((target >> 12) & 0x3F) | 0x80);
            target_buffer[2] = (char)(((target >> 6) & 0x3F) | 0x80);
            target_buffer[3] = (char)(((target >> 0) & 0x3F) | 0x80);
            target_buffer[4] = 0;

            return 4;
        }
        else
        {
            target_buffer[0] = '?';
            target_buffer[1] = 0;
        }
        return 0;
    }

    constexpr int codepoint_from_utf8(unicode_codepoint *target, const char *target_buffer)
    {
        *target = 0;
        if ((target_buffer[0] & 0xf8) == 0xf0)
        {
            *target |= ((0x07 & target_buffer[0]) << 18);
            *target |= ((0x3f & target_buffer[1]) << 12);
            *target |= ((0x3f & target_buffer[2]) << 6);
            *target |= ((0x3f & target_buffer[3]));
            return 4;
        }
        else if ((target_buffer[0] & 0xf0) == 0xe0)
        {
            *target |= ((0x0f & target_buffer[0]) << 12);
            *target |= ((0x3f & target_buffer[1]) << 6);
            *target |= ((0x3f & target_buffer[2]));
            return 3;
        }
        else if ((target_buffer[0] & 0xe0) == 0xc0)
        {
            *target |= ((0x1f & target_buffer[0]) << 6);
            *target |= ((0x3f & target_buffer[1]));
            return 2;
        }
        else
        {
            *target = target_buffer[0];
            return 1;
        }
    }

    template <typename T>
    class character_code
    {
    public:
        character_code(){};

        character_code(character_code &copy)
        {
            from_code_point(copy.to_codepoint());
        }

        character_code(const character_code &copy)
        {
            from_code_point(copy.to_codepoint());
        }

        virtual T get() const { return 0; };

        virtual bool is_null() const { return true; };

        virtual unicode_codepoint to_codepoint() const { return 0; };
        virtual void from_code_point(const unicode_codepoint v){};

        character_code &operator=(const character_code &v)
        {
            this->from_code_point(v.to_codepoint());
            return *this;
        }

        operator char()
        {
            char target[5];

            auto v = to_codepoint();
            utf8_from_codepoint(v, target);

            return target[0];
        }
    };

    class ascii_character_code : public character_code<char>
    {
        uint16_t value;

    public:
        ascii_character_code()
        {
            value = 0;
        }

        virtual char get() const { return value; };

        bool is_null() const { return value == 0; };

        virtual unicode_codepoint to_codepoint() const
        {
            unicode_codepoint ret;
            codepoint_from_utf8(&ret, (const char *)&value);
            return ret;
        };

        virtual void from_code_point(const unicode_codepoint v)
        {

            utf8_from_codepoint(v, (char *)&value);
        };
    };

    class utf8_character_code : public character_code<unicode_codepoint>
    {
        unicode_codepoint value;

    public:
        utf8_character_code()
        {
            value = 0;
        }

        virtual unicode_codepoint get() const { return value; };

        bool is_null() const { return value == 0; };

        virtual unicode_codepoint to_codepoint() const
        {
            unicode_codepoint ret;
            codepoint_from_utf8(&ret, (const char *)&value);
            return ret;
        };

        virtual void from_code_point(const unicode_codepoint v)
        {
            utf8_from_codepoint(v, (char *)&value);
        };
    };

    class unicode_character_code : public character_code<uint32_t>
    {
        uint32_t value;

    public:
        virtual uint32_t get() const { return value; };

        unicode_character_code()
        {
            value = 0;
        }

        bool is_null() const { return value == 0; };

        virtual unicode_codepoint to_codepoint() const
        {
            return value;
        };

        virtual void from_code_point(const unicode_codepoint v)
        {
            value = v;
        };
    };

    template <typename v>
    char to_char(const v value)
    {
        ascii_character_code ascii;
        ascii.from_code_point(value.to_codepoint());
        return ascii.get();
    }
    template <typename v>
    size_t u_strlen(const v *value)
    {

        size_t string_length = 0;

        while (!value[string_length].is_null())
        {
            string_length++;
        }

        return string_length;
    }

} // namespace utils

#endif // UNICODE_H
