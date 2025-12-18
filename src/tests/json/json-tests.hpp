
#pragma once

#include <libcore/fmt/log.hpp>
#include <libcore/str.hpp>

#include "../test.hpp"
#include "json/json.hpp"
#include "libcore/ds/bitmap.hpp"
#include "libcore/mem/mem.hpp"

static core::Str tjbuf = core::Str("{\"key-str\": \"value\", \"key-int\": 123, \"key-bool\": true, \"key-null\": null, \"key-array\": [1, 2, 3], \"key-object\": {\"nested-key\": \"nested-value\"}}");
static constexpr TestGroup jsonTests = {
    test_grouped_tests$("json",
                        Test(
                            "json parse",
                            []() -> Test::RetFn
                            {
                                auto json = wjson::Json::parse(tjbuf);
                                if (json.is_error())
                                {
                                    return json.error();
                                }
                                auto value = json.unwrap();
                                if (value.root().type != wjson::JsonType::Object)
                                {
                                    return "Expected object type";
                                }
                                return {};
                            }),
                        Test(
                            "json parse string",
                            []() -> Test::RetFn
                            {
                                auto json = wjson::Json::parse(tjbuf);
                                if (json.is_error())
                                {
                                    return json.error();
                                }
                                auto value = json.unwrap();
                                auto key_str = value.root().get("key-str");
                                if (key_str.is_error())
                                {
                                    return key_str.error();
                                }

                                auto val = key_str.unwrap();
                                if (val->as_string().unwrap() != core::Str("value"))
                                {
                                    return "Expected value for key-str";
                                }

                                return {};
                            }),
                        Test(
                            "json parse int",
                            []() -> Test::RetFn
                            {
                                auto json = wjson::Json::parse(tjbuf);
                                if (json.is_error())
                                {
                                    return json.error();
                                }
                                auto value = json.unwrap();
                                auto key_int = value.root().get("key-int");
                                if (key_int.is_error())
                                {
                                    return key_int.error();
                                }
                                auto val = key_int.unwrap();
                                if (val->as_number().unwrap() != 123)
                                {
                                    return "Expected 123 for key-int";
                                }
                                return {};
                            }),
                        Test(
                            "json parse bool",
                            []() -> Test::RetFn
                            {
                                auto json = wjson::Json::parse(tjbuf);
                                if (json.is_error())
                                {
                                    return json.error();
                                }
                                auto value = json.unwrap();
                                auto key_bool = value.root().get("key-bool");
                                if (key_bool.is_error())
                                {

                                    return key_bool.error();
                                }
                                auto val = key_bool.unwrap();
                                if (val->as_bool().unwrap() != true)
                                {
                                    return "Expected true for key-bool";
                                }
                                return {};
                            }),
                        Test(
                            "json parse null",
                            []() -> Test::RetFn
                            {
                                auto json = wjson::Json::parse(tjbuf);
                                if (json.is_error())
                                {
                                    return json.error();
                                }
                                auto value = json.unwrap();
                                auto key_null = value.root().get("key-null");
                                if (key_null.is_error())
                                {
                                    return key_null.error();
                                }
                                auto val = key_null.unwrap();
                                if (!val->is_null())
                                {
                                    return "Expected null for key-null";
                                }
                                return {};
                            }),
                        Test(
                            "json parse array",
                            []() -> Test::RetFn
                            {
                                auto json = wjson::Json::parse(tjbuf);
                                if (json.is_error())
                                {
                                    return json.error();
                                }
                                auto value = json.unwrap();
                                auto key_array = value.root().get("key-array");
                                if (key_array.is_error())
                                {
                                    return key_array.error();
                                }
                                auto val = key_array.unwrap();
                                if (val->type != wjson::JsonType::Array)
                                {
                                    return "Expected array type for key-array";
                                }
                                auto &arr = val->children();
                                if (arr.len() != 3 || try$(arr[0].as_number()) != 1 || try$(arr[1].as_number()) != 2 || try$(arr[2].as_number()) != 3)
                                {

                                    return "Expected array [1, 2, 3] for key-array";
                                }
                                return {};
                            }),
                        Test(
                            "json parse object",
                            []() -> Test::RetFn
                            {
                                auto json = wjson::Json::parse(tjbuf);
                                if (json.is_error())
                                {
                                    return json.error();
                                }
                                auto value = json.unwrap();
                                auto key_object = value.root().get("key-object");
                                if (key_object.is_error())
                                {
                                    return key_object.error();
                                }
                                auto val = key_object.unwrap();
                                if (val->type != wjson::JsonType::Object)
                                {
                                    return "Expected object type for key-object";
                                }
                                auto obj = val;
                                auto nested_key = obj->get("nested-key");
                                if (nested_key.is_error())
                                {
                                    return nested_key.error();
                                }
                                if (nested_key.unwrap()->as_string().unwrap() != core::Str("nested-value"))
                                {
                                    return "Expected nested-value for nested-key";
                                }
                                return {};
                            }))};