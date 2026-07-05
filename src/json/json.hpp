#pragma once

#include "libcore/ds/vec.hpp"
#include "libcore/mem/view.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"

namespace wjson
{

enum class JsonType : uint8_t
{
    Null = 0,
    Boolean,
    Number,
    String,
    Array,
    Object
};

struct JsonValue;

struct JsonChilds
{

    fc::Vec<JsonValue> values = {};
    fc::Vec<fc::Str> keys = {}; // for objects

    JsonChilds() : values(), keys() {};

    JsonChilds(JsonChilds &&other) noexcept
        : values(fc::move(other.values)), keys(fc::move(other.keys))
    {
    }

    JsonChilds(const JsonChilds &other)
        : values(other.values), keys(other.keys)
    {
    }

    JsonChilds &operator=(JsonChilds &&other) noexcept
    {
        if (this != &other)
        {
            fc::swap(values, other.values);
            fc::swap(keys, other.keys);
        }
        return *this;
    }

    // Copy assignment
    JsonChilds &operator=(const JsonChilds &other)
    {
        if (this != &other)
        {
            values = other.values;
            keys = other.keys;
        }
        return *this;
    }
};

struct JsonStorage
{
    fc::Str raw;
    bool boolean;
    double number;
    int integer;
    JsonChilds childs;

    JsonStorage() : raw(""), boolean(false), number(0), integer(0), childs() {}

    JsonStorage(JsonStorage &&other)
        : raw((other.raw)),
          boolean(other.boolean),
          number(other.number),
          integer(other.integer),
          childs(fc::move(other.childs))
    {
    }

    JsonStorage(const JsonStorage &other)
        : raw(other.raw),
          boolean(other.boolean),
          number(other.number),
          integer(other.integer),
          childs(other.childs)
    {
    }

    // Move assignment
    JsonStorage &operator=(JsonStorage &&other) noexcept
    {
        if (this != &other)
        {
            fc::swap(raw, other.raw);
            boolean = other.boolean;
            number = other.number;
            integer = other.integer;
            fc::swap(childs, other.childs);
        }
        return *this;
    }

    // Copy assignment
    JsonStorage &operator=(const JsonStorage &other)
    {
        if (this != &other)
        {
            raw = other.raw;
            boolean = other.boolean;
            number = other.number;
            integer = other.integer;
            childs = other.childs;
        }
        return *this;
    }
};

struct JsonValue
{

    JsonType type = {};

    JsonStorage storage = {};
    JsonValue() : type(JsonType::Null), storage() {};

    JsonValue(JsonValue &&other) noexcept
        : type(other.type), storage(fc::move(other.storage))
    {
        other.type = JsonType::Null;
    }

    JsonValue(const JsonValue &other)
        : type(other.type), storage(other.storage)
    {
    }

    JsonValue &operator=(JsonValue &&other)
    {
        if (this != &other)
        {
            type = other.type;
            fc::swap(storage, other.storage);
        }
        return *this;
    }

    // Copy assignment
    JsonValue &operator=(const JsonValue &other)
    {
        if (this != &other)
        {
            type = other.type;
            storage = other.storage;
        }
        return *this;
    }

    ~JsonValue()
    {
        // Storage members have their own destructors that will be called automatically
        // Vec and Str destructors handle cleanup properly
        type = JsonType::Null;
    }

    fc::Result<JsonValue> get(const fc::Str &key)
    {
        if (type != JsonType::Object)
        {
            return fc::Result<JsonValue>::error(("Not an object"));
        }
        for (size_t i = 0; i < storage.childs.keys.len(); i++)
        {
            if (storage.childs.keys[i] == key)
            {
                return storage.childs.values[i];
            }
        }
        return fc::Result<JsonValue>::error(("Key not found"));
    }

    fc::Result<JsonValue> get(size_t index)
    {
        if (type != JsonType::Array)
        {
            return fc::Result<JsonValue>::error(("Not an array"));
        }
        if (index < storage.childs.values.len())
        {
            return storage.childs.values[index];
        }
        return fc::Result<JsonValue>::error(("Index out of bounds"));
    }

    bool is_null()
    {
        return type == JsonType::Null;
    }

    fc::Result<bool> as_bool()
    {
        if (type == JsonType::Boolean)
        {
            return storage.boolean;
        }
        return fc::Result<bool>::error("Not a boolean");
    }

    fc::Result<int> as_number()
    {
        if (type == JsonType::Number)
        {
            return storage.integer;
        }
        return fc::Result<int>::error("Not a number");
    }

    fc::Result<fc::Str> as_string()
    {
        if (type == JsonType::String)
        {
            return storage.raw;
        }
        return fc::Result<fc::Str>::error("Not a string");
    }

    fc::Result<fc::Vec<JsonValue>> as_array()
    {
        if (type == JsonType::Array)
        {
            return storage.childs.values;
        }

        return fc::Result<fc::Vec<JsonValue>>::error("Not an array");
    }

    JsonValue &operator[](size_t index)
    {

        if (index < storage.childs.values.len())
        {
            return storage.childs.values[index];
        }
        while (true)
        {
        };
        return storage.childs.values[0];
    }

    JsonValue &operator[](const fc::Str &key)
    {
        for (size_t i = 0; i < storage.childs.keys.len(); i++)
        {
            if (storage.childs.keys[i] == key)
            {
                return storage.childs.values[i];
            }
        }
        while (true)
        {
        };
        return storage.childs.values[0]; // or throw an error
    }

    // Convenience getter for array children
    fc::Vec<JsonValue> &children()
    {
        return storage.childs.values;
    }
};

class Json
{

    JsonValue _root = {};
    fc::MemView<char> data;

public:
    JsonValue &root()
    {
        return _root;
    }

    static fc::Result<Json> parse(fc::MemView<char> reader);
};
}; // namespace wjson
