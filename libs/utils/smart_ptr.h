#ifndef SMART_PTR_H
#define SMART_PTR_H
#include <stddef.h>
namespace utils
{
    template <typename type>
    class unique_ptr
    {
        type *raw;

    public:
        unique_ptr() : raw(nullptr){};
        unique_ptr(type *d) : raw(d){};

        type &get()
        {
            return *raw;
        }

        type get() const
        {
            return *raw;
        }

        const type *get_raw() const
        {
            return raw;
        }

        type *get_raw()
        {
            return raw;
        }

        template <typename ret_t>
        ret_t as()
        {
            return reinterpret_cast<ret_t>(raw);
        }

        type *operator->()
        {
            return raw;
        }

        const type *operator->() const
        {
            return raw;
        }

        unique_ptr &operator=(unique_ptr &&val)
        {
            raw = val.release();
            return *this;
        }

        type &operator[](size_t idx)
        {
            return *(raw + idx);
        }

        operator bool()
        {
            return raw != nullptr;
        }

        type *release()
        {
            type *old = raw;
            raw = nullptr;
            return old;
        }

        void reset(type *newval)
        {
            type *old = raw;
            raw = newval;
            if (old)
            {
                delete old;
            }
        }

        ~unique_ptr()
        {
            if (raw)
            {
                delete raw;
            }
        }
    };

    template <typename type, typename... Args>
    unique_ptr<type> make_unique(Args... arg)
    {
        return unique_ptr<type>(new type(arg...));
    }
} // namespace utils

#endif // SMART_PTR_H
