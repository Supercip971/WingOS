#include "wchar.h"

#include "libc/locale.h"
#include "stdio.h"

size_t mbrlen(const char *s, size_t n, mbstate_t *ps)
{
    mbstate_t internal = {};
    return mbrtowc(NULL, s, n, ps != NULL ? ps : &internal);
}

static size_t _utf8ToCodePoint(const char *str, size_t len, wchar_t *res)
{
    // https://en.wikipedia.org/wiki/UTF-8
    if (len == 0)
    {
        return 0;
    }
    if ((str[0] & 0b10000000) == 0)
    {
        // 0 yyyzzzz
        *res = str[0];
        return 1;
    }
    else if ((str[0] & 0b11100000) == 0b11000000)
    {

        // 110 xxx yy | 10 yyzzzz
        if (len == 1 ||
            (str[1] & 0b11000000) != 0b10000000)
        {
            return (size_t)-1;
        }

        *res = ((str[0] & 0b00011111) << 6) | (str[1] & 0b00111111);
        return 2;
    }
    else if ((str[0] & 0b11110000) == 0b11100000)
    {

        // 1110 wwww | 10 xxxx yy | 10 yy zzzz
        if (len < 3 ||
            (str[1] & 0b11000000) != 0b10000000 ||
            (str[2] & 0b11000000) != 0b10000000)
        {
            return (size_t)-1;
        }
        //                      wwww                          xxxxyy                        yyzzzz
        *res = ((str[0] & 0b00001111) << 12) | ((str[1] & 0b00111111) << 6) | (str[2] & 0b00111111);
        return 3;
    }
    else if ((str[0] & 0b11111000) == 0b11110000)
    {

        // 11110 uvv | 10 vv wwww | 10 xxxx yy | 10 yy zzzz
        if (len < 4 ||
            (str[1] & 0b11000000) != 0b10000000 ||
            (str[2] & 0b11000000) != 0b10000000 ||
            (str[3] & 0b11000000) != 0b10000000)
        {
            return (size_t)-1;
        }

        //                       uvv                          vvwwww                          xxxxyy                        zzzzzz
        *res = ((str[0] & 0b00000111) << 18) | ((str[1] & 0b00111111) << 12) | ((str[2] & 0b00111111) << 6) | (str[3] & 0b00111111);
        return 4;
    }
    else
    {
        return (size_t)-1;
    }
}

size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps)
{
    if (getencoding() != LC_WOS_UTF8)
    {
        printf("warning: mbrtowc: encoding not supported, only UTF-8 is supported\n");
        // only support UTF-8 for now
        return (size_t)-1;
    }

    if (s == NULL)
    {
        // reset state
        if (ps != NULL)
        {
            ps->state = 0;
        }
        return 0;
    }

    wchar_t codepoint;
    size_t bytes_read = _utf8ToCodePoint(s, n, &codepoint);

    if (bytes_read == (size_t)-1)
    {
        return (size_t)-1;
    }

    if (pwc != NULL)
    {
        *pwc = codepoint;
    }

    return bytes_read;
}
