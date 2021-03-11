#ifndef MATH_H
#define MATH_H
namespace utils
{

    template <class A>
    constexpr A max(const A a, const A b)
    {
        if (a > b)
        {
            return a;
        }
        else
        {
            return b;
        }
    }

    template <class A>
    constexpr A min(const A a, const A b)
    {
        if (a < b)
        {
            return a;
        }
        else
        {
            return b;
        }
    }

} // namespace utils

#endif // MATH_H
