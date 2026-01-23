#pragma once

#include <libcore/fmt/log.hpp>
#include <libcore/str.hpp>
#include <libcore/result.hpp>

#include "../test.hpp"

// Helper class to track construction/destruction for Result tests
struct ResultTestObject
{
    int value;
    static int construct_count;
    static int copy_construct_count;
    static int move_construct_count;
    static int copy_assign_count;
    static int move_assign_count;
    static int destruct_count;

    ResultTestObject(int v = 0) : value(v)
    {
        construct_count++;
    }

    ResultTestObject(const ResultTestObject& other) : value(other.value)
    {
        copy_construct_count++;
        construct_count++;
    }

    ResultTestObject(ResultTestObject&& other) : value(other.value)
    {
        move_construct_count++;
        construct_count++;
    }

    ResultTestObject& operator=(const ResultTestObject& other)
    {
        copy_assign_count++;
        value = other.value;
        return *this;
    }

    ResultTestObject& operator=(ResultTestObject&& other)
    {
        move_assign_count++;
        value = other.value;
        return *this;
    }

    ~ResultTestObject()
    {
        destruct_count++;
    }

    static void reset_counts()
    {
        construct_count = 0;
        copy_construct_count = 0;
        move_construct_count = 0;
        copy_assign_count = 0;
        move_assign_count = 0;
        destruct_count = 0;
    }
};

int ResultTestObject::construct_count = 0;
int ResultTestObject::copy_construct_count = 0;
int ResultTestObject::move_construct_count = 0;
int ResultTestObject::copy_assign_count = 0;
int ResultTestObject::move_assign_count = 0;
int ResultTestObject::destruct_count = 0;

// Error type for tracking error construction/destruction
struct ResultTestError
{
    int code;
    static int construct_count;
    static int copy_construct_count;
    static int move_construct_count;
    static int copy_assign_count;
    static int move_assign_count;
    static int destruct_count;

    ResultTestError(int c = 0) : code(c)
    {
        construct_count++;
    }

    ResultTestError(const ResultTestError& other) : code(other.code)
    {
        copy_construct_count++;
        construct_count++;
    }

    ResultTestError(ResultTestError&& other) : code(other.code)
    {
        move_construct_count++;
        construct_count++;
    }

    ResultTestError& operator=(const ResultTestError& other)
    {
        copy_assign_count++;
        code = other.code;
        return *this;
    }

    ResultTestError& operator=(ResultTestError&& other)
    {
        move_assign_count++;
        code = other.code;
        return *this;
    }

    ~ResultTestError()
    {
        destruct_count++;
    }

    static void reset_counts()
    {
        construct_count = 0;
        copy_construct_count = 0;
        move_construct_count = 0;
        copy_assign_count = 0;
        move_assign_count = 0;
        destruct_count = 0;
    }

    // Allow conversion to const char* for Result error messages
    operator const char*() const
    {
        return "test error";
    }
};

int ResultTestError::construct_count = 0;
int ResultTestError::copy_construct_count = 0;
int ResultTestError::move_construct_count = 0;
int ResultTestError::copy_assign_count = 0;
int ResultTestError::move_assign_count = 0;
int ResultTestError::destruct_count = 0;

// Add format_v for ResultTestError to support logging
template <core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, const ResultTestError &err)
{
    return fmt::format(target, "error({})", err.code);
}

static constexpr TestGroup resultTests = {
    test_grouped_tests$(
        "result",
        Test(
            "basic success creation",
            []() -> Test::RetFn {
                core::Result<int, const char*> result = 42;
                
                if (result.is_error())
                    return "result should not be error";
                if (result.unwrap() != 42)
                    return "result value should be 42";
                
                return {};
            }),
        Test(
            "basic error creation",
            []() -> Test::RetFn {
                core::Result<int, const char*> result = "error message";
                
                if (!result.is_error())
                    return "result should be error";
                if (core::Str(result.error()) != core::Str("error message"))
                    return "error message mismatch";
                
                return {};
            }),
        Test(
            "success static factory",
            []() -> Test::RetFn {
                auto result = core::Result<int, const char*>::success(100);
                
                if (result.is_error())
                    return "result should not be error";
                if (result.unwrap() != 100)
                    return "result value should be 100";
                
                return {};
            }),
        Test(
            "error static factory",
            []() -> Test::RetFn {
                auto result = core::Result<int, const char*>::error("test error");
                
                if (!result.is_error())
                    return "result should be error";
                if (core::Str(result.error()) != core::Str("test error"))
                    return "error message mismatch";
                
                return {};
            }),
        Test(
            "bool operator on success",
            []() -> Test::RetFn {
                core::Result<int, const char*> result = 42;
                
                if (!(bool)result)
                    return "bool operator should be true for success";
                if (!(result ? true : false))
                    return "ternary operator should be true for success";
                
                return {};
            }),
        Test(
            "bool operator on error",
            []() -> Test::RetFn {
                core::Result<int, const char*> result = "error";
                
                if ((bool)result)
                    return "bool operator should be false for error";
                if (result ? true : false)
                    return "ternary operator should be false for error";
                
                return {};
            }),
        Test(
            "take() moves value",
            []() -> Test::RetFn {
                core::Result<int, const char*> result = 42;
                
                int value = result.take();
                if (value != 42)
                    return "take() should return 42";
                
                return {};
            }),
        Test(
            "value construction tracking",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                
                {
                    ResultTestObject obj(10);
                    if (ResultTestObject::construct_count != 1)
                        return "construct_count should be 1";
                    
                    core::Result<ResultTestObject, const char*> result = core::move(obj);
                    // Constructor + move constructor
                    if (ResultTestObject::construct_count != 2)
                        return "construct_count should be 2 after move to result";
                    if (ResultTestObject::move_construct_count != 1)
                        return "move_construct_count should be 1";
                }
                
                // All objects should be destroyed
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                
                return {};
            }),
        Test(
            "error construction tracking",
            []() -> Test::RetFn {
                ResultTestError::reset_counts();
                
                {
                    ResultTestError err(404);
                    if (ResultTestError::construct_count != 1)
                        return "construct_count should be 1";
                    
                    core::Result<int, ResultTestError> result = core::move(err);
                    // Constructor + move constructor
                    if (ResultTestError::construct_count != 2)
                        return "construct_count should be 2 after move to result";
                    if (ResultTestError::move_construct_count != 1)
                        return "move_construct_count should be 1";
                }
                
                // All errors should be destroyed
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "all errors should be destroyed";
                
                return {};
            }),
        Test(
            "move constructor with success",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                
                {
                    core::Result<ResultTestObject, const char*> result1 = ResultTestObject(42);
                    if (ResultTestObject::construct_count != 2)
                        return "construct_count should be 2";
                    
                    core::Result<ResultTestObject, const char*> result2 = core::move(result1);
                    if (ResultTestObject::construct_count != 3)
                        return "construct_count should be 3 after move constructor";
                    if (ResultTestObject::move_construct_count != 2)
                        return "move_construct_count should be 2";
                    
                    if (result2.is_error())
                        return "result2 should not be error";
                    if (result2.unwrap().value != 42)
                        return "result2 value should be 42";
                }
                
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                
                return {};
            }),
        Test(
            "move constructor with error",
            []() -> Test::RetFn {
                ResultTestError::reset_counts();
                
                {
                    core::Result<int, ResultTestError> result1 = ResultTestError(500);
                    if (ResultTestError::construct_count != 2)
                        return "construct_count should be 2";
                    
                    core::Result<int, ResultTestError> result2 = core::move(result1);
                    if (ResultTestError::construct_count != 3)
                        return "construct_count should be 3 after move constructor";
                    if (ResultTestError::move_construct_count != 2)
                        return "move_construct_count should be 2";
                    
                    if (!result2.is_error())
                        return "result2 should be error";
                    if (result2.error().code != 500)
                        return "result2 error code should be 500";
                }
                
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "all errors should be destroyed";
                
                return {};
            }),
        Test(
            "move assignment with success",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                
                {
                    core::Result<ResultTestObject, const char*> result1 = ResultTestObject(10);
                    core::Result<ResultTestObject, const char*> result2 = ResultTestObject(20);
                    
                    int before_construct = ResultTestObject::construct_count;
                    int before_destruct = ResultTestObject::destruct_count;
                    
                    result2 = core::move(result1);
                    
                    // One destruction (old value in result2) + one construction (move)
                    if (ResultTestObject::destruct_count != before_destruct + 1)
                        return "destruct_count should increase by 1";
                    if (ResultTestObject::construct_count != before_construct + 1)
                        return "construct_count should increase by 1";
                    
                    if (result2.is_error())
                        return "result2 should not be error";
                    if (result2.unwrap().value != 10)
                        return "result2 value should be 10";
                }
                
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                
                return {};
            }),
        Test(
            "move assignment with error",
            []() -> Test::RetFn {
                ResultTestError::reset_counts();
                
                {
                    core::Result<int, ResultTestError> result1 = ResultTestError(100);
                    core::Result<int, ResultTestError> result2 = ResultTestError(200);
                    
                    int before_construct = ResultTestError::construct_count;
                    int before_destruct = ResultTestError::destruct_count;
                    
                    result2 = core::move(result1);
                    
                    // One destruction (old error in result2) + one construction (move)
                    if (ResultTestError::destruct_count != before_destruct + 1)
                        return "destruct_count should increase by 1";
                    if (ResultTestError::construct_count != before_construct + 1)
                        return "construct_count should increase by 1";
                    
                    if (!result2.is_error())
                        return "result2 should be error";
                    if (result2.error().code != 100)
                        return "result2 error code should be 100";
                }
                
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "all errors should be destroyed";
                
                return {};
            }),
        Test(
            "move assignment from error to success",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                ResultTestError::reset_counts();
                
                {
                    core::Result<ResultTestObject, ResultTestError> result1 = ResultTestObject(42);
                    core::Result<ResultTestObject, ResultTestError> result2 = ResultTestError(404);
                    
                    if (result1.is_error())
                        return "result1 should not be error";
                    if (!result2.is_error())
                        return "result2 should be error";
                    
                    result2 = core::move(result1);
                    
                    if (result2.is_error())
                        return "result2 should not be error after move";
                    if (result2.unwrap().value != 42)
                        return "result2 value should be 42";
                }
                
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "all errors should be destroyed";
                
                return {};
            }),
        Test(
            "move assignment from success to error",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                ResultTestError::reset_counts();
                
                {
                    core::Result<ResultTestObject, ResultTestError> result1 = ResultTestError(500);
                    core::Result<ResultTestObject, ResultTestError> result2 = ResultTestObject(99);
                    
                    if (!result1.is_error())
                        return "result1 should be error";
                    if (result2.is_error())
                        return "result2 should not be error";
                    
                    result2 = core::move(result1);
                    
                    if (!result2.is_error())
                        return "result2 should be error after move";
                    if (result2.error().code != 500)
                        return "result2 error code should be 500";
                }
                
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "all errors should be destroyed";
                
                return {};
            }),
        Test(
            "self assignment",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                
                {
                    core::Result<ResultTestObject, const char*> result = ResultTestObject(123);
                    
                    int before_construct = ResultTestObject::construct_count;
                    int before_destruct = ResultTestObject::destruct_count;
                    
                    result = core::move(result);
                    
                    // Self assignment should not create or destroy anything
                    if (ResultTestObject::construct_count != before_construct)
                        return "self assignment should not construct";
                    if (ResultTestObject::destruct_count != before_destruct)
                        return "self assignment should not destruct";
                    
                    if (result.is_error())
                        return "result should not be error";
                    if (result.unwrap().value != 123)
                        return "result value should be 123";
                }
                
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                
                return {};
            }),
        Test(
            "void result success",
            []() -> Test::RetFn {
                core::Result<void, const char*> result;
                
                if (result.is_error())
                    return "void result should not be error";
                if (!(bool)result)
                    return "void result bool operator should be true";
                
                return {};
            }),
        Test(
            "void result error",
            []() -> Test::RetFn {
                core::Result<void, const char*> result = "void error";
                
                if (!result.is_error())
                    return "void result should be error";
                if (core::Str(result.error()) != core::Str("void error"))
                    return "error message mismatch";
                
                return {};
            }),
        Test(
            "void result move constructor",
            []() -> Test::RetFn {
                ResultTestError::reset_counts();
                
                {
                    core::Result<void, ResultTestError> result1 = ResultTestError(123);
                    core::Result<void, ResultTestError> result2 = core::move(result1);
                    
                    if (!result2.is_error())
                        return "result2 should be error";
                    if (result2.error().code != 123)
                        return "result2 error code should be 123";
                }
                
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "all errors should be destroyed";
                
                return {};
            }),
        Test(
            "void result move assignment",
            []() -> Test::RetFn {
                ResultTestError::reset_counts();
                
                {
                    core::Result<void, ResultTestError> result1 = ResultTestError(100);
                    core::Result<void, ResultTestError> result2 = ResultTestError(200);
                    
                    result2 = core::move(result1);
                    
                    if (!result2.is_error())
                        return "result2 should be error";
                    if (result2.error().code != 100)
                        return "result2 error code should be 100";
                }
                
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "all errors should be destroyed";
                
                return {};
            }),
        Test(
            "no memory leaks on multiple reassignments",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                
                {
                    core::Result<ResultTestObject, const char*> result = ResultTestObject(1);
                    
                    result = ResultTestObject(2);
                    result = ResultTestObject(3);
                    result = ResultTestObject(4);
                    result = ResultTestObject(5);
                    
                    if (result.is_error())
                        return "result should not be error";
                    if (result.unwrap().value != 5)
                        return "result value should be 5";
                }
                
                // All objects should be properly destroyed
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                
                return {};
            }),
        Test(
            "destructor called on error type",
            []() -> Test::RetFn {
                ResultTestError::reset_counts();
                
                {
                    core::Result<int, ResultTestError> result = ResultTestError(999);
                    if (!result.is_error())
                        return "result should be error";
                }
                
                // Error destructor should have been called
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "error destructor should be called";
                
                return {};
            }),
        Test(
            "destructor called on value type",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                
                {
                    core::Result<ResultTestObject, const char*> result = ResultTestObject(888);
                    if (result.is_error())
                        return "result should not be error";
                }
                
                // Value destructor should have been called
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "value destructor should be called";
                
                return {};
            }),
        Test(
            "unwrap returns reference",
            []() -> Test::RetFn {
                core::Result<int, const char*> result = 100;
                
                int& ref = result.unwrap();
                ref = 200;
                
                if (result.unwrap() != 200)
                    return "unwrap should return reference";
                
                return {};
            }),
        Test(
            "unwrap rvalue returns rvalue reference",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                
                {
                    auto make_result = []() {
                        return core::Result<ResultTestObject, const char*>(ResultTestObject(42));
                    };
                    
                    ResultTestObject obj = make_result().unwrap();
                    if (obj.value != 42)
                        return "unwrapped value should be 42";
                }
                
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                
                return {};
            }),
        Test(
            "complex destruction order",
            []() -> Test::RetFn {
                ResultTestObject::reset_counts();
                ResultTestError::reset_counts();
                
                {
                    core::Result<ResultTestObject, ResultTestError> r1 = ResultTestObject(1);
                    core::Result<ResultTestObject, ResultTestError> r2 = ResultTestError(2);
                    core::Result<ResultTestObject, ResultTestError> r3 = ResultTestObject(3);
                    core::Result<ResultTestObject, ResultTestError> r4 = ResultTestError(4);
                    
                    // Mix of success and error states
                    if (r1.is_error())
                        return "r1 should not be error";
                    if (!r2.is_error())
                        return "r2 should be error";
                    if (r3.is_error())
                        return "r3 should not be error";
                    if (!r4.is_error())
                        return "r4 should be error";
                }
                
                // All should be properly destroyed
                if (ResultTestObject::construct_count != ResultTestObject::destruct_count)
                    return "all objects should be destroyed";
                if (ResultTestError::construct_count != ResultTestError::destruct_count)
                    return "all errors should be destroyed";
                
                return {};
            })
    )
};