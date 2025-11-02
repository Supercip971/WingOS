#include "string.h"

void *memcpy(void *dest, const void *src, size_t n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    
    // Copy word-by-word for better performance when aligned
    if (((size_t)d & (sizeof(size_t) - 1)) == 0 && 
        ((size_t)s & (sizeof(size_t) - 1)) == 0)
    {
        size_t *wd = (size_t *)d;
        const size_t *ws = (const size_t *)s;
        while (n >= sizeof(size_t))
        {
            *wd++ = *ws++;
            n -= sizeof(size_t);
        }
        d = (char *)wd;
        s = (const char *)ws;
    }
    
    // Copy remaining bytes
    while (n--)
    {
        *d++ = *s++;
    }
    return dest;
}
void *memset(void *s, int c, size_t n)
{
    char *p = (char *)s;
    
    // For large blocks and aligned addresses, use word-sized operations
    if (n >= sizeof(size_t) && ((size_t)p & (sizeof(size_t) - 1)) == 0)
    {
        // Create a word-sized pattern
        size_t pattern = (unsigned char)c;
        pattern |= pattern << 8;
        pattern |= pattern << 16;
        #if __SIZEOF_SIZE_T__ == 8
        pattern |= pattern << 32;
        #endif
        
        size_t *wp = (size_t *)p;
        while (n >= sizeof(size_t))
        {
            *wp++ = pattern;
            n -= sizeof(size_t);
        }
        p = (char *)wp;
    }
    
    // Set remaining bytes
    while (n--)
    {
        *p++ = (char)c;
    }
    return s;
}