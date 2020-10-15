#include <utility.h>
// not used and not tested
char *strtok(char *s, const char *delm)
{
    static int currIndex = 0;
    if (!s || !delm || s[currIndex] == '\0' || s[currIndex + 1] == '\0')
    {
        return 0;
    }
    char *W = (char *)malloc(sizeof(char) * 100);
    for (int i = 0; i < 100; i++)
    {
        W[i] = 0;
    }

    int i = currIndex;
    int k = 0;
    int j = 0;

    while (s[i] != '\0')
    {
        j = 0;
        while (delm[j] != '\0')
        {
            if (s[i] != delm[j])
            {
                W[k] = s[i];
            }
            else
            {
                W[i] = 0;
                currIndex = i + 1;
                return W;
            }
            j++;
        }

        i++;
        k++;
    }
    W[i] = 0;
    currIndex = i + 1;
    return W;
}

int isdigit(int c)
{
    if (c <= '9' && c >= '0')
    {
        return 1;
    }
    return 0;
}

int64_t strtoint(const char *nptr)
{

    const char *s = nptr;
    long long ret_value = 0;
    uint8_t base = 10;
    bool is_neg = false;
    while (*s == ' ')
    {
        s++;
    }
    if (*s == '-')
    {
        is_neg = true;
        s++;
    }

    while (isdigit(*s))
    {
        ret_value = (*s - '0') + ret_value * base;

        s++;
    }

    if (is_neg)
    {
        return ret_value * -1;
    }
    return ret_value;
}

void kitoa(char *buf, int base, int d)
{
    char *p = buf;
    char *p1, *p2;
    int64_t ud = d;
    int64_t divisor = 10;

    if (base == 'd' && d < 0)
    {
        *p++ = '-';
        buf++;
        ud = -d;
        divisor = 10;
    }
    else if (base == 'x')
    {
        divisor = 16;
    }

    do
    {
        int64_t remainder = ud % divisor;

        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);

    *p = 0;
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2)
    {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

void kitoa64(char *buf, int base, int64_t d)
{
    char *p = buf;
    char *p1, *p2;
    int64_t ud = d;
    int64_t divisor = 10;

    if (base == 'd' && d < 0)
    {
        *p++ = '-';
        buf++;
        ud = -d;
        divisor = 10;
    }
    else if (base == 'x')
    {
        divisor = 16;
    }

    do
    {
        int64_t remainder = ud % divisor;

        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);

    *p = 0;
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2)
    {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

template <>
void kitoaT(char *buf, int base, unsigned long d)
{
    char *p = buf;
    char *p1, *p2;
    unsigned long ud = d;
    unsigned long divisor = 10;

    if (base == 'd' && d < 0)
    {
        *p++ = '-';
        buf++;
        ud = -d;
        divisor = 10;
    }
    else if (base == 'x')
    {
        divisor = 16;
    }

    do
    {
        unsigned long remainder = ud % divisor;

        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);

    *p = 0;
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2)
    {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}
int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2))
    {
        ++s1;
        ++s2;
        --n;
    }
    if (n == 0)
    {
        return 0;
    }
    return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++, s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

size_t strlen(const char *s)
{
    size_t i = 0;

    while (s[i] != 0)
    {
        i++;
    }
    return i;
}

extern "C" void __cxa_pure_virtual()
{
    log("virtual", LOG_ERROR) << "error while trying to call a virtual function D:";
}
