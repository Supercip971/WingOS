#include <ctypes.h>
#include <kern/mem_util.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/liballoc.h>
#include <utils/string_util.h>

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
int abs(int j)
{
    if (j < 0)
    {
        return -j;
    }
    return j;
}

long labs(long j)
{
    if (j < 0)
    {
        return -j;
    }
    return j;
}

long long llabs(long long j)
{
    if (j < 0)
    {
        return -j;
    }
    return j;
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
#ifdef __SSE__
double atof(const char *s)
{
    double result = 0.0;
    int offset = 0;
    int temp_char = 0;
    while (isdigit(temp_char = *s++))
    {
        result = result * 10 + (temp_char - '0');
    }
    if (temp_char == '.')
    {
        while (isdigit(temp_char = *s++))
        {
            result = result * 10 + (temp_char - '0');
        }
        offset = offset - 1;
    }
    else
    {
        return result;
    }

    while (offset < 0)
    {
        result *= 0.1;
        offset += 1;
    }
    return result;
}

double strtod(const char *nptr, char **endptr)
{
    const char *s = nptr;
    while (*s == ' ')
    {
        s++;
    }
    bool is_neg = false;
    if (*s == '-')
    {
        is_neg = true;
        s++;
    }

    const char *d2 = s + 1;
    double res = atof(d2);
    if (is_neg == true)
    {
        res *= -1;
    }
    if (endptr != 0)
    {
        *endptr = (char *)d2;
    }
    return res;
}
#endif
long long strtoll(const char *nptr, char **endptr, int base)
{

    const char *s = nptr;
    long long ret_value = 0;
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
    // if base == 0 and detect 0x at the start of string put it at 16
    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
        s += 2;
        base = 16;
    }
    if (base == 0)
    {
        base = 10;
    }

    char *start = (char *)s;
    while (isalnum(*s))
    {
        if (base == 10)
        {
            if (isdigit(*s))
            {
                ret_value = (*s - '0') * base;
            }
            else
            {
                break;
            }
        }
        else if (base == 16)
        {
            if (isdigit(*s))
            {

                ret_value = (*s - '0') * base;
            }
            else
            {
                ret_value = ((to_lower(*s) - 'a') + 10) * base;
            }
        }
        s++;
    }
    if (is_neg)
    {
        ret_value *= -1;
    }
    if (endptr != 0)
    {
        *endptr = start;
    }
    return ret_value;
}
long strtol(const char *nptr, char **endptr, int base)
{

    const char *s = nptr;
    long ret_value = 0;
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
    // if base == 0 and detect 0x at the start of string put it at 16
    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
        s += 2;
        base = 16;
    }
    if (base == 0)
    {
        base = 10;
    }

    char *start = (char *)s;
    while (isalnum(*s))
    {
        if (base == 10)
        {
            if (isdigit(*s))
            {
                ret_value = (*s - '0') * base;
            }
            else
            {
                break;
            }
        }
        else if (base == 16)
        {
            if (isdigit(*s))
            {

                ret_value = (*s - '0') * base;
            }
            else
            {
                ret_value = ((to_lower(*s) - 'a') + 10) * base;
            }
        }
        s++;
    }
    if (is_neg)
    {
        ret_value *= -1;
    }
    if (endptr != 0)
    {
        *endptr = start;
    }
    return ret_value;
}

extern "C" void __cxa_pure_virtual()
{
    printf("error __cxa_pure_virtual() called");
}

unsigned long int cseed = 1;

int rand()
{
    cseed = cseed * 1103515245 + 12345;
    return (unsigned int)(cseed / 65536) % 32768;
}

void srand(uint32_t seed)
{
    cseed = seed;
}

void *malloc(size_t size)
{
    return wos::wmalloc(size);
}

void free(void *ptr)
{
    wos::wfree(ptr);
}

void *realloc(void *ptr, size_t size)
{
    return wos::wrealloc(ptr, size);
}
