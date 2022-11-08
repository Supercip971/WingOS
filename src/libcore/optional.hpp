#pragma once 

#include <libcore/type-utils.hpp>

#include <stddef.h>
namespace core 
{


    template <class T>
    union Storage 
    {
        bool _dummy; 
        T _value;

        constexpr Storage() : _dummy(false) {}
        constexpr Storage(const T &value) : _value(value) {}
        constexpr Storage(T &&value) : _value(core::move(value)) {}


       // template<typename ...Args>
       // constexpr Storage(Args&&... args) : _value(core::forward(args)...) {}
    };

    template <class T>
    class Optional 
    {
        bool _contain_value;
        Storage<T> _value;
        public: 
        constexpr Optional() : _contain_value(false) {}
        constexpr Optional(const T &value) : _contain_value(true), _value(value) {}
        constexpr Optional(T &&value) : _contain_value(true), _value(core::move(value)) {}

        constexpr Optional(const Optional &other) : _contain_value(other._contain_value)
        {
            if (_contain_value)
            {
                _value._value = other._value._value;
            }
        }
        constexpr Optional(Optional &&other) : _contain_value(other._contain_value) {

            if(other._contain_value) {
                _value._value = core::move(other._value._value);
            }
        }

        constexpr Optional &operator=(const Optional &other) 
        {

            if(other.has_value() && _contain_value)
            {
                _value._value = (*other);
            }
            else if(other.has_value() && !_contain_value)
            {
                _value._value = core::move((*other));
                other._contain_value = false;
                _contain_value = true;
                
            }
            else if(!other.has_value() && _contain_value)
            {
                _contain_value = false;
                _value._value.~T();
            }           
            return *this;
        }

        constexpr Optional &operator=(Optional &&other) 
        {

            if(other.has_value() && _contain_value)
            {
                _value._value = core::move(other._value._value);
            }
            else if(other.has_value() && !_contain_value)
            {
                _value._value = core::move(other._value._value);
                _contain_value = true;
                
            }
            else if(!other.has_value() && _contain_value)
            {
                _contain_value = false;
                _value._value.~T();
            }
            return *this;
        }

        constexpr bool has_value() const 
        {
            return _contain_value;
        }

        constexpr T &value() 
        {
            return _value._value;
        }

        constexpr const T &value() const 
        {
            return _value._value;
        }

        constexpr T &operator*() 
        {
            return _value._value;
        }

        constexpr const T &operator*() const 
        {
            return _value._value;
        }

        constexpr T *operator->() 
        {
            return &_value._value;
        }

        constexpr const T *operator->() const 
        {
            return &_value._value;
        }

        constexpr T &&unwrap() 
        {
            _contain_value = false;
            return core::move(_value._value);
        }

        constexpr ~Optional() 
        {
            if (_contain_value) 
            {
                _value._value.~T();
            }
            _contain_value = false;
        }
    };


    
}