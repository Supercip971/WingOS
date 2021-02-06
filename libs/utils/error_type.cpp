#include "error_type.h"

namespace wos
{
    error_type::error_type()
    {
        value = error_class::SUCCESS;
    }
    error_type::error_type(int type)
    {
        value = (error_class)type;
    }
    error_type::error_type(const char *msg, error_class type, int subtype)
    {

        value = type;
        this->msg = msg;
        this->subtype = subtype;
    }
} // namespace wos
