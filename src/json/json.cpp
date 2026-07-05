#include "json.hpp"

#include "libcore/fmt/log.hpp"
#include "libcore/mem/view.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "parser/scanner.hpp"

namespace wjson
{

fc::Result<JsonValue> parse_json_value(fc::Scanner<char> &scanner)
{
    JsonValue result{};
    if (scanner.skip_spaced('{').unwrap())
    {

        result.type = JsonType::Object;
        if (scanner.skip_spaced('}').unwrap())
        {
            return result;
        }

        // parse object
        while (!scanner.ended())
        {

            scanner.skip_spaces();
            if (!try$(scanner.skip_spaced('"')))
            {
                return fc::Result<JsonValue>::error("Expected '\"' at the start of key");
            }
            auto key = try$(scanner.read_until('"'));
            if (!try$(scanner.skip_spaced('"')))
            {
                return fc::Result<JsonValue>::error("Expected '\"' at the end of key");
            }
            scanner.skip_spaces();

            if (!try$(scanner.skip_spaced(':')))
            {
                return fc::Result<JsonValue>::error("Expected ':' after key");
            }

            auto val = try$(parse_json_value(scanner));

            result.storage.childs.keys.push(fc::Str(key));
            result.storage.childs.values.push(fc::move(val));

            if (!try$(scanner.skip_spaced(',')))
            {
                break; // end of object
            }
        }
        scanner.skip_spaces();

        if (!try$(scanner.skip('}')))
        {

            return fc::Result<JsonValue>::error("Expected '}' at the end of object");
        }
        return result;
    }
    else if (scanner.skip_spaced('[').unwrap())
    {
        if (scanner.skip_spaced(']').unwrap())
        {

            JsonValue empty_array = {};
            empty_array.type = JsonType::Array;

            return fc::Result<JsonValue>(empty_array);
        }
        // parse array
        while (!scanner.ended())
        {

            // parse object

            auto val = try$(parse_json_value(scanner));
            scanner.skip_spaces();

            result.storage.childs.values.push(fc::move(val));

            if (!scanner.skip_spaced(',').unwrap())
            {
                break; // end of object
            }
        }

        if (!scanner.skip_spaced(']').unwrap())
        {
            return fc::Result<JsonValue>("Expected ']' at the end of object");
        }
        result.type = JsonType::Array;

        return result;
    }
    else if (scanner.skip_string(fc::Str("true")).unwrap())
    {
        result.type = JsonType::Boolean;
        result.storage.boolean = true;
        return result;
    }
    else if (scanner.skip_string(fc::Str("false")).unwrap())
    {
        result.type = JsonType::Boolean;
        result.storage.boolean = false;
        return result;
    }
    else if (scanner.skip_string(fc::Str("null")).unwrap())
    {
        result.type = JsonType::Null;
        return result;
    }
    else if (scanner.skip_spaced('"').unwrap())
    {
        auto str = try$(scanner.read_until('"'));
        scanner.skip('"');
        result.type = JsonType::String;
        result.storage.raw = fc::Str(str);
        return result;
    }

    auto integer = (scanner.skip_int());

    if (integer.is_error())
    {
        fmt::log$("error parsing integer: {}", integer.error());
        fmt::log$("remaining: {}", scanner.read_until('\0').unwrap());
        return fc::Result<JsonValue>(integer.error());
    }
    result.type = JsonType::Number;
    result.storage.integer = integer.unwrap();

    return result;
}

fc::Result<Json> Json::parse(fc::MemView<char> reader)
{

    Json json{};

    fc::Scanner<char> scanner(reader);
    json._root = try$(parse_json_value(scanner));
    return json;
}

} // namespace wjson
