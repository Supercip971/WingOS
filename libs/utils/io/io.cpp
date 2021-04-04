#include "io.h"
#include <stdio.h>
namespace utils
{
    size_t basic_seeker_implementation::seek(long target_index, size_t whence)
    {
        if (whence == SEEK_SET)
        {
            if (target_index < (long)_send)
            {
                _scur = target_index;
                return _scur;
            }
            else
            {
                printf("seek: trying to seek oob with seek_set %i >= %i \n", target_index, _send);
                return -1;
            }
        }
        else if (whence == SEEK_END)
        {
            if (((long)_send - 1) + target_index < (long)_send)
            {
                _scur = (_send - 1) + target_index;
                return _scur;
            }
            else
            {

                printf("seek: trying to seek oob with seek_end %i >= %i \n", (_send - 1) + target_index, _send);
                return -1;
            }
        }
        else if (whence == SEEK_CUR)
        {
            if ((long)(_scur) + target_index < (long)_send)
            {
                _scur = _scur + target_index;
                return _scur;
            }
            else
            {

                printf("seek: trying to seek oob with seek_cur %i >= %i \n", _scur + target_index, _send);
                return -1;
            }
        }
        printf("seek: trying to seek with a invalid whence value: %i \n", whence);
        return -1;
    }
} // namespace utils
