#include <com.h>
#include <loggging.h>
loggging::loggging()
{
}
void loggging::set_log_type(const char *data, log_state log_state)
{
    if (log_state == 0)
    {
        printf("\n\033[36m [ %s ] :\033[0m ", data);
    }
    else if (log_state == 1)
    {

        printf("\n\n\033[1m\033[32m [ %s ] :\033[0m ", data);
    }
    else if (log_state == 2)
    {
        printf("\n\033[31;1m [ %s ] :\033[0m ", data);
    }
    else if (log_state == LOG_WARNING)
    {
        printf("\n\033[31m [ %s ] :", data);
    }
    else
    {
        printf("\n\033[4m\033[1m\033[31m [ FATAL ] [ %s ] : [0m", data);
    }
}
loggging loggging::operator<<(const char *string)
{
    printf(string);
    return *this;
}
loggging loggging::operator<<(uint64_t address)
{
    printf(" %x ", address);
    return *this;
}
loggging main_logging_system;
loggging log(const char *data, log_state color_mode)
{

    main_logging_system.set_log_type(data, color_mode);
    return main_logging_system;
}
