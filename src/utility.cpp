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
    //Iterator = ++ptr;
    return W;
}
