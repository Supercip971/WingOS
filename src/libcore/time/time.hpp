#pragma once 


namespace core {

    template<unsigned long scale> 
    struct DurationUnit {

        
        long _value = 0; 


        constexpr long value() const {
            return _value;
        }
        static constexpr unsigned long Scale = scale;

        constexpr DurationUnit() = default;
        constexpr DurationUnit(long value) : _value(value) {}

        constexpr DurationUnit<scale> operator+(const DurationUnit<scale>& other) const {
            return DurationUnit<scale>(_value + other._value);
        }

        constexpr DurationUnit<scale> operator-(const DurationUnit<scale>& other) const {
            return DurationUnit<scale>(_value - other._value);
        }

        constexpr DurationUnit<scale> operator*(long factor) const {
            return DurationUnit<scale>(_value * factor);
        }

        constexpr DurationUnit<scale> operator/(long divisor) const {
            return DurationUnit<scale>(_value / divisor);
        }

        template<unsigned long other_scale>
        constexpr DurationUnit<other_scale> to() const {
            if constexpr (Scale == other_scale) {
                return DurationUnit<other_scale>(_value);
            } else if constexpr (Scale < other_scale) {
                unsigned long factor = other_scale / Scale;
                return DurationUnit<other_scale>(_value / factor);  // DIVIDE when going to larger unit
            } else {
                unsigned long factor = Scale / other_scale;
                return DurationUnit<other_scale>(_value * factor);  // MULTIPLY when going to smaller unit
            }
        }

        template<typename T> 
        constexpr T to() const {
            return to<T::Scale>();
        }


        template<unsigned long other_scale>
        constexpr DurationUnit(const DurationUnit<other_scale>& other) {
            _value = other.template to<Scale>().value();
        }

        constexpr bool operator<=(const DurationUnit& other) const {
            return _value <= other._value;
        }
        constexpr bool operator>=(const DurationUnit& other) const {
            return _value >= other._value;
        }

        constexpr bool operator<(const DurationUnit& other) const {
            return _value < other._value;
        }

        constexpr bool operator>(const DurationUnit& other) const {
            return _value > other._value;
        }



        
    };


    using Nanoseconds = DurationUnit<1>;
    using Microseconds = DurationUnit<Nanoseconds::Scale * 1000>;
    using Milliseconds = DurationUnit<Microseconds::Scale * 1000>;
    using Seconds = DurationUnit<Milliseconds::Scale * 1000>;
    using Minutes = DurationUnit<Seconds::Scale * 60>;
    using Hours = DurationUnit<Minutes::Scale * 60>;
    using Days = DurationUnit<Hours::Scale * 24>;

}