#ifndef NO_NULL_H
#define NO_NULL_H

#include <assert.h>
#include <stddef.h>
#include <utils/type/is_same.h>
namespace utils
{
    template <typename T>
    class no_null
    {
        T _value;

    public:
        static_assert(!is_same<nullptr_t, T>::value, "no null must not be null");
        template <typename T2>
        no_null(T2 val)
        {
            static_assert(!is_same<nullptr_t, T2>::value, "no null constructor must not be null");
            assert(reinterpret_cast<uintptr_t>(val) != 0);
            _value = val;
        }
        no_null(T val)
        {
            assert(reinterpret_cast<uintptr_t>(val) != 0);
            _value = val;
        }
        template <typename T2>
        no_null &operator=(T2 val)
        {
            static_assert(!is_same<nullptr_t, T2>::value, "no null constructor must not be null");
            assert(reinterpret_cast<uintptr_t>(val) != 0);
            _value = val;
        }

        remove_pointer<T> operator->()
        {
            return _value;
        }

        T get() { return _value; };
    };

} // namespace utils
#endif // NO_NULL_H
