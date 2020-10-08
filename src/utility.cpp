#include <utility.h>
// not used and not tested
char *strtok(char *s, const char *delm)
{
    static int currIndex = 0;
    if (!s || !delm || s[currIndex] == '\0' || s[currIndex + 1] == '\0')
        return 0;
    char *W = (char *)malloc(sizeof(char) * 100);
    for (int i = 0; i < 100; i++)
    {
        W[i] = 0;
    }
    int i = currIndex, k = 0, j = 0;

    while (s[i] != '\0')
    {
        j = 0;
        while (delm[j] != '\0')
        {
            if (s[i] != delm[j])
                W[k] = s[i];
            else
                goto It;
            j++;
        }

        i++;
        k++;
    }
It:
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
    const char *end;
    long long ret_value = 0;
    char *end_buffer;
    int current_error;
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
    // if base == 0 and detect 0x at the start of string put it at 16

    char *start = (char *)s;
    while (isdigit(*s))
    {
        ret_value = (*s - '0') + ret_value * base;

        s++;
    }
    if (is_neg)
    {
        ret_value *= -1;
    }
    return ret_value;
}
