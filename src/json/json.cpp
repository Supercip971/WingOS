#include "json.hpp"

#include "parser/scanner.hpp"
namespace wjson
{

core::Result<JsonValue> parse_json_value(core::Scanner<char> &scanner)
{
    JsonValue result;

    if (scanner.skip_spaced('{').unwrap())
    {

        result.type = JsonType::Object;
        if (scanner.skip_spaced('}').unwrap())
        {
            return core::Result<JsonValue>(JsonValue{JsonType::Object, JsonStorage{}, {}, {}});
        }

        // parse object
        while (!scanner.ended())
        {

            scanner.skip_spaces();
            if (!try$(scanner.skip_spaced('"')))
            {
                return core::Result<JsonValue>("Expected '\"' at the start of key");
            }
            auto key = try$(scanner.read_until('"'));
            if (!try$(scanner.skip_spaced('"')))
            {
                return core::Result<JsonValue>("Expected '\"' at the end of key");
            }
            scanner.skip_spaces();

            if (!try$(scanner.skip_spaced(':')))
            {
                return core::Result<JsonValue>("Expected ':' after key");
            }

            auto val = try$(parse_json_value(scanner));

            result.keys.push(core::Str(key));
            result.children.push(core::move(val));

            if (!try$(scanner.skip_spaced(',')))
            {
                break; // end of object
            }
        }
        scanner.skip_spaces();

        if (!try$(scanner.skip('}')))
        {

            return core::Result<JsonValue>("Expected '}' at the end of object");
        }
        return result;
    }
    else if (scanner.skip_spaced('[').unwrap())
    {
        if (scanner.skip_spaced(']').unwrap())
        {
            return core::Result<JsonValue>(JsonValue{JsonType::Array, JsonStorage{}, {}, {}});
        }
        // parse array
        while (!scanner.ended())
        {

            // parse object

            auto val = try$(parse_json_value(scanner));
            scanner.skip_spaces();

            result.children.push(core::move(val));

            if (!scanner.skip_spaced(',').unwrap())
            {
                break; // end of object
            }
        }

        if (!scanner.skip_spaced(']').unwrap())
        {
            return core::Result<JsonValue>("Expected ']' at the end of object");
        }
        result.type = JsonType::Array;

        return result;
    }
    else if (scanner.skip_string(core::Str("true")).unwrap())
    {
        result.type = JsonType::Boolean;
        result.storage.boolean = true;
        return result;
    }
    else if (scanner.skip_string(core::Str("false")).unwrap())
    {
        result.type = JsonType::Boolean;
        result.storage.boolean = false;
        return result;
    }
    else if (scanner.skip_string(core::Str("null")).unwrap())
    {
        result.type = JsonType::Null;
        return result;
    }
    else if (scanner.skip_spaced('"').unwrap())
    {
        auto str = try$(scanner.read_until('"'));
        scanner.skip('"');
        result.type = JsonType::String;
        result.storage.raw = core::Str(str);
        return result;
    }

    auto integer = (scanner.skip_int());

    if (integer.is_error())
    {
        log::log$("error parsing integer: {}", integer.error());
        log::log$("remaining: {}", scanner.read_until('\0').unwrap());
        return core::Result<JsonValue>(integer.error());
    }
    result.type = JsonType::Number;
    result.storage.integer = integer.unwrap();

    return result;
}
core::Result<Json> Json::parse(core::MemView<char> reader)
{

    Json json;

    core::Scanner<char> scanner(reader);
    json._root = try$(parse_json_value(scanner));
    return json;
}

} // namespace wjson