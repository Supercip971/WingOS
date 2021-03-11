#pragma once

namespace utils
{
    enum class error_class : int
    {
        SUCCESS = 1,
        ERROR = 0
    };

    class error_type
    {
    public:
        error_type(); // return just success
        error_type(int type);
        error_type(const char *msg, error_class type, int subtype = 0);

        operator bool() const { return value == error_class::SUCCESS; };

        operator const char *() const { return msg; };

        const char *get_error_message()
        {
            return msg;
        }

        int get_sub_type()
        {
            return subtype;
        }

    private:
        error_class value;
        int subtype;
        const char *msg;
    };

} // namespace utils
