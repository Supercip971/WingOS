#ifndef IS_SAME_H
#define IS_SAME_H
#include <utils/type/integral_constant.h>
#include <utils/type_traits.h>
namespace utils
{
    template <class T1, class T2>
    class is_same : public false_type
    {
    };

    template <class T1>
    class is_same<T1, T1> : public true_type
    {
    };

} // namespace fth
#endif // IS_SAME_H
