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
    
    template <class A>
    constexpr A abs(const A a)
    {
        if(a < 0)
        {
            return -a;
        }
        
        return a;
    }

} // namespace utils

#endif // MATH_H
