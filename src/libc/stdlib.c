#include "stdlib.h"
#include "liballoc/liballoc.h"
#include "ctype.h"

void *malloc(size_t size)
{
    return kmalloc(size);
}
void free(void *ptr)
{
    kfree(ptr);
}
void *realloc(void *ptr, size_t size)
{
    return krealloc(ptr, size);
}

long strtol(const char *nptr, char **endptr, int base)
{
    return strtoll(nptr, endptr, base);
}


/*
 The  strtol()  function  converts the initial part of the string in nptr to a long
       integer value according to the given base, which must be between 2 and  36  inclu‐
       sive, or be the special value 0.

       The string may begin with an arbitrary amount of white space (as determined by is‐
       space(3))  followed  by a single optional '+' or '-' sign.  If base is zero or 16,
       the string may then include a "0x" or "0X" prefix, and the number will be read  in
       base  16; if base is zero or 2, the string may then include a "0b" or "0B" prefix,
       and the number will be read in base 2; otherwise, a zero base is taken as 10 (dec‐
       imal) unless the next character is '0', in which case it is taken as 8 (octal).

       The remainder of the string is converted to a long value in  the  obvious  manner,
       stopping at the first character which is not a valid digit in the given base.  (In
       bases above 10, the letter 'A' in either uppercase or lowercase represents 10, 'B'
       represents 11, and so forth, with 'Z' representing 35.)

       If  endptr  is not NULL, and the base is supported, strtol() stores the address of
       the first invalid character in *endptr.  If there were no digits at all,  strtol()
       stores  the  original value of nptr in *endptr (and returns 0).  In particular, if
       *nptr is not '\0' but **endptr is '\0' on return, the entire string is valid.

       The strtoll() function works just like the strtol() function but  returns  a  long
       long integer value.

*/
long long strtoll(const char *nptr, char **endptr, int base)
{
    if(base < 2 || base > 36)
    {
        if (endptr)
        {
            *endptr = (char *)nptr;
        }
        return 0;
    }


    const char *s = nptr;
    while (isspace((unsigned char)*s))
        s++;

    int sign = 1;
    if (*s == '-')
    {
        sign = -1;
        s++;
    }

    else if (*s == '+')
    {
        s++;
    }

    if(*s == '0')
    {
        s++;
        if((*s == 'x' || *s == 'X') && (base == 0 || base == 16))
        {
            s++;
            base = 16;
        }
        else if((*s == 'b' || *s == 'B') && (base == 0 || base == 2))
        {
            s++;
            base = 2;
        }
        else if(base == 0)
        {
            base = 8;
        }
    }

    if(base == 0)
    {
        base = 10;
    }

    long long result = 0;
    const char *start = s;
    while (1)
    {
        int digit;
        if (*s >= '0' && *s <= '9')
        {
            digit = *s - '0';
        }
        else if (*s >= 'a' && *s <= 'z')
        {
            digit = *s - 'a' + 10;
        }
        else if (*s >= 'A' && *s <= 'Z')
        {
            digit = *s - 'A' + 10;
        }
        else
        {
            break;
        }
        if (digit >= base)
        {
            break;
        }

        result = result * base + digit;
        s++;
    }

    if (endptr)
    {
        if (s == start)
        {
            *endptr = (char *)nptr;
        }
        else
        {
            *endptr = (char *)s;
        }
    }

    return sign * result;

}

#ifndef __ck_kernel__
// FIXME: support exponent
double strtod(const char* nptr, char** endptr)
{
    const char* s = nptr;
    while (isspace((unsigned char)*s))
        s++;

    int sign = 1;
    if (*s == '-')
    {
        sign = -1;
        s++;
    }
    else if (*s == '+')
    {
        s++;
    }

    double result = 0.0;
    while (*s >= '0' && *s <= '9')
    {
        result = result * 10.0 + (*s - '0');
        s++;
    }

    if (*s == '.')
    {
        s++;
        double frac = 0.0;
        double base = 0.1;
        while (*s >= '0' && *s <= '9')
        {
            frac += (*s - '0') * base;
            base *= 0.1;
            s++;
        }
        result += frac;
    }

    if (endptr)
    {
        *endptr = (char *)s;
    }

    return sign * result;
}


double atof(const char* str)
{
    char* endptr;
    double val = strtod(str, &endptr);
    return val;
}

#endif 
int atoi(const char* str)
{
    return (int)strtol(str, NULL, 10);
}