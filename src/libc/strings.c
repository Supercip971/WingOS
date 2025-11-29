#include "strings.h"


int strcasecmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        char c1 = *s1;
        char c2 = *s2;

        // Convert to lowercase if uppercase
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += ('a' - 'A');
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += ('a' - 'A');

        if (c1 != c2)
            return (unsigned char)c1 - (unsigned char)c2;

        s1++;
        s2++;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        char c1 = s1[i];
        char c2 = s2[i];

        // If we reach the end of either string, stop comparison
        if (c1 == '\0' || c2 == '\0')
            return (unsigned char)c1 - (unsigned char)c2;

        // Convert to lowercase if uppercase
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += ('a' - 'A');
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += ('a' - 'A');

        if (c1 != c2)
            return (unsigned char)c1 - (unsigned char)c2;
    }

    return 0; // Strings are equal up to n characters
}
