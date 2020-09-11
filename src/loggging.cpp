#include <com.h>
#include <loggging.h>
loggging::loggging()
{
}
void loggging::set_log_type(const char *data, log_state log_state)
{
    if (log_state == 0)
    {
        printf("\n\033[34m [ %s ] :\033[37m ", data);
    }
    else if (log_state == 1)
    {
        printf("\n\033[32m [ %s ] :\033[37m ", data);
    }
    else
    {
        printf("\n\033[31m [ %s ] :\033[37m ", data);
    }
}
loggging loggging::operator<<(const char *string)
{
    printf(string);
    return *this;
}
loggging loggging::operator<<(uint64_t address)
{
    printf("%x", address);
    return *this;
}
loggging main_logging_system;
loggging log(const char *data, log_state color_mode)
{

    main_logging_system.set_log_type(data, color_mode);
    return main_logging_system;
}
