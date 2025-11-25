#include <ctype.h>

int isalnum(int c)
{
    return (isalpha(c) || (c >= '0' && c <= '9'));
}
int isalpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}
int isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r');
}