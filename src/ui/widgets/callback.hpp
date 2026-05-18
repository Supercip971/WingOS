#pragma once


#include "ui/widgets/widget.hpp"
namespace fc
{

    class Callback
    {



        public:

            Widget * linked;
            using CallbackType = void(Widget*);

            CallbackType *callback;

            Callback(CallbackType *callback) : callback(callback) {}
            Callback() : callback(nullptr) {}

            Callback(Widget* linked, CallbackType* callback ) :  linked(linked), callback(callback) {}


            void call()
            {
                if (callback != nullptr)
                {
                    callback(linked);
                }
            }

            template<typename T, typename W>
            static Callback construct(T callback, W* linked)
            {
                return Callback((Widget*)linked, (CallbackType*)callback);
            }
    };


    #define AutoCallback$(fn) Callback::construct([](fc::Widget* w){\
        auto* casted = static_cast<decltype(this)>(w);\
        fn(casted); \
        }, this)
}
