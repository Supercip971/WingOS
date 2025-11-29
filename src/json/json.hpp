#pragma once

#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/mem/view.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
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

    core::Vec<JsonValue> values;
    core::Vec<core::Str> keys; // for objects

    JsonChilds() : values(), keys() {};

    JsonChilds(JsonChilds&& other) noexcept
        : values(core::move(other.values)), keys(core::move(other.keys))
    {
    }

    JsonChilds(const JsonChilds& other)
        : values(other.values), keys(other.keys)
    {
    }

    JsonChilds& operator=(JsonChilds&& other) noexcept
    {
        if (this != &other)
        {
            values = core::move(other.values);
            keys = core::move(other.keys);
        }
        return *this;
    }

    // Copy assignment
    JsonChilds& operator=(const JsonChilds& other)
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
    core::Str raw;
    bool boolean;
    double number;
    int integer;
    JsonChilds childs;

    JsonStorage() : raw(""), boolean(false), number(0), integer(0), childs() {}

    JsonStorage(JsonStorage&& other) noexcept
        : raw(core::move(other.raw)), 
          boolean(other.boolean),
          number(other.number),
          integer(other.integer),
          childs(core::move(other.childs))
    {
    }

    JsonStorage(const JsonStorage& other)
        : raw(other.raw),
          boolean(other.boolean),
          number(other.number),
          integer(other.integer),
          childs(other.childs)
    {
    }

    // Move assignment
    JsonStorage& operator=(JsonStorage&& other) noexcept
    {
        if (this != &other)
        {
            raw = core::move(other.raw);
            boolean = other.boolean;
            number = other.number;
            integer = other.integer;
            childs = core::move(other.childs);
        }
        return *this;
    }

    // Copy assignment
    JsonStorage& operator=(const JsonStorage& other)
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

    JsonType type;

    JsonStorage storage;
    JsonValue() : type(JsonType::Null), storage() {};

    JsonValue(JsonValue&& other) noexcept
        : type(other.type), storage(core::move(other.storage))
    {
        other.type = JsonType::Null;
    }

    JsonValue(const JsonValue& other)
        : type(other.type), storage(other.storage)
    {
    }

    JsonValue& operator=(JsonValue&& other) noexcept
    {
        if (this != &other)
        {
            type = other.type;
            storage = core::move(other.storage);
            other.type = JsonType::Null;
        }
        return *this;
    }

    // Copy assignment
    JsonValue& operator=(const JsonValue& other)
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

    core::Result<JsonValue *> get(const core::Str &key)
    {
        if (type != JsonType::Object)
        {
            return core::Result<JsonValue *>::error(("Not an object"));
        }
        for (size_t i = 0; i < storage.childs.keys.len(); i++)
        {
            if (storage.childs.keys[i] == key)
            {
                return &storage.childs.values[i];
            }
        }
        return core::Result<JsonValue *>::error(("Key not found"));
    }

    core::Result<JsonValue *> get(size_t index)
    {
        if (type != JsonType::Array)
        {
            return core::Result<JsonValue *>::error(("Not an array"));
        }
        if (index < storage.childs.values.len())
        {
            return &storage.childs.values[index];
        }
        return core::Result<JsonValue *>::error(("Index out of bounds"));
    }

    bool is_null()
    {
        return type == JsonType::Null;
    }

    core::Result<bool> as_bool()
    {
        if (type == JsonType::Boolean)
        {
            return storage.boolean;
        }
        return core::Result<bool>::error("Not a boolean");
    }

    core::Result<int> as_number()
    {
        if (type == JsonType::Number)
        {
            return storage.integer;
        }
        return core::Result<int>::error("Not a number");
    }

    core::Result<core::Str> as_string()
    {
        if (type == JsonType::String)
        {
            return storage.raw;
        }
        return core::Result<core::Str>::error("Not a string");
    }

    core::Result<core::Vec<JsonValue> *> as_array()
    {
        if (type == JsonType::Array)
        {
            return &storage.childs.values;
        }
        return core::Result<core::Vec<JsonValue> *>::error("Not an array");
    }

    JsonValue *operator[](size_t index)
    {

        if (index < storage.childs.values.len())
        {
            return &storage.childs.values[index];
        }
        while (true)
        {
        };
        return &storage.childs.values[0];
    }
    JsonValue *operator[](const core::Str &key)
    {
        for (size_t i = 0; i < storage.childs.keys.len(); i++)
        {
            if (storage.childs.keys[i] == key)
            {
                return &storage.childs.values[i];
            }
        }
        while (true)
        {
        };
        return &storage.childs.values[0]; // or throw an error
    }
};

class Json
{

    JsonValue _root = {};
    core::MemView<char> data;

public:
    JsonValue &root()
    {
        return _root;
    }

    static core::Result<Json> parse(core::MemView<char> reader);
};
}; // namespace wjson