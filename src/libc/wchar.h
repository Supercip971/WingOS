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

    wchar_t *wcschr(const wchar_t *wcs, wchar_t wc);

    size_t mbsrtowcs(wchar_t *d, const char **s, size_t l, mbstate_t *ps);

    int iswspace(wchar_t wc);

#ifdef __cplusplus
}
#endif
