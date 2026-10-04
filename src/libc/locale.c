#include <string.h>

#include "locale.h"
#include "stdio.h"

char *setlocale(int category, const char *locale)
{
    (void)category;
    (void)locale;

    if (strcmp(locale, "en_US.UTF-8") == 0 || strcmp(locale, "C") == 0 || strcmp(locale, "POSIX") == 0)
    {
        return (char *)locale;
    }
    else
    {
        printf("warning: setlocale: locale '%s' not supported, falling back to 'C'\n", locale);

        return NULL;
    }
}

int getencoding()
{
    return LC_WOS_UTF8;
}
