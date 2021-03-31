#ifndef ERROR_RESULT_H
#define ERROR_RESULT_H
#include <assert.h>
#include <utils/wstring.h>
namespace utils
{

    class error
    {
        const char *_error = "no error"; // don't use utils::string for the moment because we need to check error as a compile time too and string is not constexpr for the moment
        const char *_file = "no file";
        int _line = 0;
        bool _is_error = false;
        bool _is_detailed = false;

    public:
        error(const char *file, int line, const char *error_str) : _error(error_str), _file(file), _line(line), _is_error(true), _is_detailed(true){};
        error(const char *error_str) : _error(error_str), _file("none"), _line(0), _is_error(true), _is_detailed(false){};
        error() = default;
        const char *error_str() const { return _error; };
        const char *file() const { return _file; };
        int line() const { return _line; };

        void throw_if_error(); // don't throw anything if not an error
        bool is_error() const { return _is_error; };

        operator bool() const
        {
            return is_error();
        }
    };

#define ERROR(str) utils::error(__FILE__, __LINE__, str)
#define NO_ERROR() utils::error()

    template <typename T>
    class error_or_result
    {
        error _possible_error;
        T _value; // todo: add a optional<T> type

    public:
        error_or_result(T value)
            : _value(value)
        {
            _possible_error = error();
        }
        error_or_result(const error err) : _value(0)
        {
            _possible_error = err;
        }
        bool is_error() const
        {
            return _possible_error.is_error();
        }

        T get_value()
        {
            _possible_error.throw_if_error();
            return _value;
        }

        error get_error() const
        {
            return _possible_error;
        }
    };
} // namespace utils
#endif // ERROR_RESULT_H
