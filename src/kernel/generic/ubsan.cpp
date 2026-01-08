
#include <libcore/fmt/log.hpp>
#include <stdint.h>
#include <libcore/lock/lock.hpp>

struct source_location
{
    const char *file;
    uint32_t line;
    uint32_t column;
};

struct type_descriptor
{
    uint16_t kind;
    uint16_t info;
    char name[];
};

struct type_mismatch_info
{
    source_location location;
    type_descriptor *type;
    uint8_t alignment;
    uint8_t type_check_kind;
};

struct out_of_bounds_info
{
    source_location location;
    type_descriptor* left_type;
    type_descriptor* right_type;
};

struct type_mismatch_data
{
    source_location loc;
    type_descriptor *type;
    unsigned char alignment;
    unsigned char type_check_kind;
};

struct overflow_data
{
    source_location loc;
    type_descriptor *type;
};

struct shift_out_of_bounds_data
{
    source_location loc;
    type_descriptor *lhs_type;
    type_descriptor *rhs_type;
};

struct out_of_bounds_data
{
    source_location loc;
    type_descriptor *array_type;
    type_descriptor *index_type;
};

struct unreachable_data
{
    source_location loc;
};

struct vla_bound_data
{
    source_location loc;
    type_descriptor *type;
};

struct invalid_value_data
{
    source_location loc;
    type_descriptor *type;
};

struct nonnull_arg_data
{
    source_location loc;
};

core::Lock locker = {};
void dump_source_location(source_location *loc)
{
    // In a real kernel, you would look up the file and line number in debug info.
    // Here, we just print the raw information.
    log::err$(" at {}:{}:{}", loc->file, loc->line, loc->column);
}

static const char* __kinds[] = {
    "load of", "store to", "reference binding to", "member access within", "member call on", "constructor call on", "downcast of", "downcast of", "upcast of", "cast to virtual base of",
};
void dump_type_descriptor(type_descriptor *type)
{
    log::err$("Type descriptor: {}", type->name);

    if(type->kind < sizeof(__kinds) / sizeof(__kinds[0]))
    {
        log::err$("Kind: {}", __kinds[type->kind]);
    }
    log::err$("Info: {}", type->info);
}

extern "C" void __ubsan_handle_function_type_mismatch(type_mismatch_data *data,
                                             unsigned long ptr __attribute__((unused)))
{

    lock_scope$(locker);
    if(!ptr)
    {
        log::err$("UBSan func type mismatch");
        dump_source_location(&data->loc);
    }
    else if(ptr & ((1 << data->alignment) - 1 ) && data->alignment != 0)
    {
        log::err$("UBSan func type alignment mismatch");
        dump_source_location(&data->loc);
    }
    else
    {
        log::err$("UBSan func type mismatch (no space for object)");
        dump_source_location(&data->loc);
    }


  //  dump_source_location(&data->loc);
  //  log::err$("UBSan func type mismatch");
}
//#error https://wiki.osdev.org/Undefined_Behavior_Sanitization
extern "C" void __ubsan_handle_type_mismatch(type_mismatch_data *data,
                                             unsigned long ptr __attribute__((unused)))
{

    lock_scope$(locker);
    if(!ptr)
    {
        log::log$("UBSan type mismatch (null)");
        dump_source_location(&data->loc);
        dump_type_descriptor(data->type);
    }
    else if((ptr & ((1 << data->alignment) - 1 )) && data->alignment != 0)
    {
        log::log$("UBSan type alignment mismatch");
        dump_source_location(&data->loc);
        dump_type_descriptor(data->type);

    }
    else
    {
        log::log$("UBSan type mismatch (no space for object)");
        dump_source_location(&data->loc);
        dump_type_descriptor(data->type);
    }

}
extern "C" void __ubsan_handle_type_mismatch_v1(type_mismatch_data *data,
                                             unsigned long ptr __attribute__((unused)))
{

    lock_scope$(locker);
    if(!ptr)
    {
        log::err$("(v1) UBSan type mismatch");
        dump_source_location(&data->loc);
        dump_type_descriptor(data->type);
    }
    else if((ptr & ((1 << data->alignment) - 1 )) && data->alignment != 0)
    {
        log::err$("(v1) UBSan type alignment mismatch");
        log::err$("Alignment expected: ", data->alignment);
        log::err$("Alignment actual: ", ptr & ((1 << data->alignment) - 1 ));

        dump_source_location(&data->loc);
        dump_type_descriptor(data->type);
    }
    else
    {
        log::err$("(v1) UBSan type mismatch (no space for object)");
        dump_source_location(&data->loc);
        dump_type_descriptor(data->type);
    }

    (void)data;
  //  dump_source_location(&data->loc);
  //  log::err$("UBSan type mismatch");
}
extern "C" void __ubsan_handle_pointer_overflow(overflow_data *data,
                                 unsigned long lhs __attribute__((unused)),
                                 unsigned long rhs __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan ptr overflow");
}
extern "C" void __ubsan_handle_add_overflow(overflow_data *data,
                                 unsigned long lhs __attribute__((unused)),
                                 unsigned long rhs __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan add overflow");
}

extern "C" void __ubsan_handle_sub_overflow(overflow_data *data,
                                 unsigned long lhs __attribute__((unused)),
                                 unsigned long rhs __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan sub overflow");
}

extern "C" void __ubsan_handle_mul_overflow(overflow_data *data,
                                 unsigned long lhs __attribute__((unused)),
                                 unsigned long rhs __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan mul overflow");
}

extern "C" void __ubsan_handle_negate_overflow(overflow_data *data,
                                    unsigned long old_val
                                    __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan negate overflow");
}

extern "C" void __ubsan_handle_divrem_overflow(overflow_data *data,
                                    unsigned long lhs __attribute__((unused)),
                                    unsigned long rhs __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan divrem overflow");
}

extern "C" void __ubsan_handle_shift_out_of_bounds(shift_out_of_bounds_data *data,
                                        unsigned long lhs
                                        __attribute__((unused)),
                                        unsigned long rhs
                                        __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan shift out of bounds");
}

extern "C" void __ubsan_handle_out_of_bounds(out_of_bounds_data *data,
                                  unsigned long idx __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan out of bounds");
}

extern "C" void __ubsan_handle_builtin_unreachable(unreachable_data *data)
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan builtin unreachable reached");
    asm volatile("int $3");
    while(true){};
}
extern "C" void __ubsan_handle_unreachable(unreachable_data *data)
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan unreachable reached");
}

extern "C" void __attribute__((noreturn))
__ubsan_handle_missing_return(unreachable_data *data)
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan missing return");
    while (true)
        ;
}

extern "C" void __ubsan_handle_vla_bound_not_positive(vla_bound_data *data,
                                           unsigned long bound
                                           __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan VLA bound not positive");
}

extern "C" void __ubsan_handle_load_invalid_value(invalid_value_data *data,
                                       unsigned long val
                                       __attribute__((unused)))
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan load invalid value");
}
extern "C" void __ubsan_handle_nonnull_return_v1(nonnull_arg_data *data)
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan nonnull return");
}

extern "C" void __ubsan_handle_nonnull_arg(nonnull_arg_data *data
#if defined __GCC_VERSION && __GCC_VERSION < 60000
                                ,
                                size_t arg_no __attribute__((unused))
#endif
)
{
    lock_scope$(locker);
    dump_source_location(&data->loc);
    log::err$("UBSan nonnull argument passed null");
}
