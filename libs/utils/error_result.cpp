
#include "error_result.h"
#include <stdlib.h>

namespace utils
{

    void error::throw_if_error()
    {
        if (is_error())
        {
            printf("\n error: %s \n", _error);
            if (_is_detailed)
            {
                printf(" in file: %s \n", _file);
                printf(" line: %i \n", _line);
            }
            exit(-1);
        }
    }
} // namespace utils
