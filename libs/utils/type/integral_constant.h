#ifndef INTEGRAL_CONSTANT_H
#define INTEGRAL_CONSTANT_H

namespace utils
{

    template <class T, T val>
    class integral_constant
    {
    public:
        static constexpr T value = val;
        using val_type = T;
        using this_type = integral_constant;
        constexpr operator val_type() const noexcept { return value; };
        constexpr val_type operator()() const noexcept { return value; };
    };

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

} // namespace fth

#endif // INTEGRAL_CONSTANT_H
