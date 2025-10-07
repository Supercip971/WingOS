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

struct JsonStorage
{
    core::Str raw;
    bool boolean = false;
    double number = 0.0;
    int integer;
};

struct JsonValue
{

    JsonType type;

    JsonStorage storage;
    core::Vec<JsonValue> children;
    core::Vec<core::Str> keys; // for objects


    ~JsonValue() {
        if(type == JsonType::String) {
            storage.raw = core::Str();
        }
        else if(type == JsonType::Array)
        {
            children.release();
        }
        else if(type == JsonType::Object)
        {
            children.release();
            keys.release();
        }
        type = JsonType::Null;
    }

    core::Result<JsonValue *> get(const core::Str &key)
    {
        if (type != JsonType::Object)
        {
            return core::Result<JsonValue *>(("Not an object"));
        }
        for (size_t i = 0; i < keys.len(); i++)
        {
            if (keys[i] == key)
            {
                return &children[i];
            }
        }
        return core::Result<JsonValue *>(("Key not found"));
    }

    core::Result<JsonValue *> get(size_t index)
    {
        if (type != JsonType::Array)
        {
            return core::Result<JsonValue *>(("Not an array"));
        }
        if (index < children.len())
        {
            return &children[index];
        }
        return core::Result<JsonValue *>(("Index out of bounds"));
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
        return core::Result<bool>("Not a boolean");
    }

    core::Result<int> as_number()
    {
        if (type == JsonType::Number)
        {
            return storage.integer;
        }
        return core::Result<int>("Not a number");
    }

    core::Result<core::Str> as_string()
    {
        if (type == JsonType::String)
        {
            return storage.raw;
        }
        return core::Result<core::Str>("Not a string");
    }

    core::Result<core::Vec<JsonValue> *> as_array()
    {
        if (type == JsonType::Array)
        {
            return &children;
        }
        return core::Result<core::Vec<JsonValue> *>("Not an array");
    }

    JsonValue *operator[](size_t index)
    {
        if (index < children.len())
        {
            return &children[index];
        }
        return nullptr; // or throw an error
    }
    JsonValue *operator[](const core::Str &key)
    {
        for (size_t i = 0; i < keys.len(); i++)
        {
            if (keys[i] == key)
            {
                return &children[i];
            }
        }
        return nullptr; // or throw an error
    }
};

class Json
{

    JsonValue _root;
    core::MemView<char> data;

public:
    JsonValue &root()
    {
        return _root;
    }

    static core::Result<Json> parse(core::MemView<char> reader);
};
}; // namespace wjson