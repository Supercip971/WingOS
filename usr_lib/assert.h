#ifndef ASSERT_H
#define ASSERT_H
#include <stdio.h>
#define assert(expression)                                          \
    if ((expression) == 0)                                          \
    {                                                               \
        printf("(ASSERT) on line %i file %s ", __LINE__, __FILE__); \
    };
#endif // ASSERT_H
