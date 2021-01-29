#include <liballoc.h>
#include <utility.h>
// not used and not tested
extern "C"
{
    void *__dso_handle;

#define ATEXIT_MAX_FUNCS 128

    struct atexit_func_entry_t
    {
        void (*destructor_func)(void *);
        void *obj_ptr;
        void *dso_handle;
    };

    typedef unsigned uarch_t;
    atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
    uarch_t __atexit_func_count = 0;

    int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
    {

        if (__atexit_func_count >= ATEXIT_MAX_FUNCS)
        {
            return -1;
        };
        __atexit_funcs[__atexit_func_count].destructor_func = f;
        __atexit_funcs[__atexit_func_count].obj_ptr = objptr;
        __atexit_funcs[__atexit_func_count].dso_handle = dso;
        __atexit_func_count++;
        return 0; /*I would prefer if functions returned 1 on success, but the ABI says...*/
    };

    void __cxa_finalize(void *f)
    {
        uarch_t i = __atexit_func_count;
        if (!f)
        {

            while (i--)
            {
                if (__atexit_funcs[i].destructor_func)
                {

                    (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
                };
            };
            return;
        };

        while (i--)
        {
            if (__atexit_funcs[i].destructor_func == f)
            {
                (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
                __atexit_funcs[i].destructor_func = 0;
            };
        };
    };
}

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

void kitoaT(char *buf, int base, size_t d)
{
    char *p = buf;
    char *p1, *p2;
    size_t ud = d;
    size_t divisor = 10;

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
        size_t remainder = ud % divisor;

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
inline int isalnum(int c)
{
    if (c <= 'z' && c >= 'a')
    {
        return 1;
    }
    if (c <= 'Z' && c >= 'A')
    {
        return 1;
    }
    if (c <= '9' && c >= '0')
    {
        return 1;
    }
    return 0;
}
inline int to_lower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        c -= 'A';
        c += 'a';
    }
    return c;
}

inline int to_upper(int c)
{
    if (c >= 'a' && c <= 'z')
    {
        c -= 'a';
        c += 'A';
    }
    return c;
}
long atoi(const char *S)
{
    long num = 0;

    int i = 0;

    // run till we have reached end of the string or
    // current character is non-numeric
    while (S[i] && (S[i] >= '0' && S[i] <= '9'))
    {
        num = num * 10 + (S[i] - '0');
        i++;
    }

    return num;
}
