#pragma once

#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct mbstate
    {
        int state;
    } mbstate_t;

    size_t mbrlen(const char *s, size_t n, mbstate_t *ps);

    size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);

#ifdef __cplusplus
}
#endif
