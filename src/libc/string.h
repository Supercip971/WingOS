#ifndef STRING_H
#define STRING_H
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void *memcpy(void *__restrict dest, const void *__restrict src, size_t n);
    void *memset(void *s, int c, size_t n);

    char *strchr(const char *s, int c);

    size_t strlen(const char *s);

    char *strerror(int errnum);

    char *strtok(char * __restrict str, const char * __restrict delim);
    char *strtok_r(char * __restrict str, const char * __restrict delim,
                   char **__restrict saveptr);


    char *strpbrk(const char *s, const char *accept);

#ifdef __cplusplus
}
#endif

#endif