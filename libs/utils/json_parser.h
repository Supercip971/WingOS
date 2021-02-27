#ifndef JSON_PARSER_H
#define JSON_PARSER_H
#include "wvector.h"
#include <stdint.h>
namespace utils
{
    enum json_type
    {
        JSON_INT = 1,
        JSON_BOOL,
        JSON_FLOAT,
        JSON_STRING,
        JSON_NULL
    };
    class json_value
    {
        bool is_loaded = false;
        json_type type;
        const char *vdata;
        void detect_type();

    public:
        bool is_valid()
        {
            return is_loaded;
        }

        void init(const char *data);

        template <typename T>
        T get_as();
        template <bool>
        bool get_as();
        template <const char *>
        const char *get_as();
        bool is_array();

        json_type get_type()
        {
            return type;
        }
    };
    class json_storage
    {
        json_value val;

    public:
        const char *storage_name;
        const char *storage_value;
        bool is_an_array;
        json_storage *parent;
        vector<json_storage *> sub_storage;
        json_value get_value();
        json_storage &operator[](const char *name);
    };

    class json_data
    {

    public:
        char get_next_data_close_delimitor(int index);
        void read();
        void from_file(const char *file_path);
        void from_data(const char *data); // null terminated string

        json_storage get_storage()
        {
            return storage;
        }

    private:
        void create_higher_storage();
        char *json_raw_data;

        json_storage storage;
        vector<char> higher_storage;
    };
} // namespace utils

#endif // JSON_PARSER_H
