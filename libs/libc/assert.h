#ifndef ASSERT_H
#define ASSERT_H
#include <stdio.h>
#include <stdlib.h>

#define assert(expression)                                          \
    if ((expression) == 0)                                          \
    {                                                               \
        printf("(ASSERT) on line %i file %s ", __LINE__, __FILE__); \
        exit(-1);                                                   \
    };
#endif // ASSERT_H
