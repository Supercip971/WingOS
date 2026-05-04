#pragma once


namespace fc
{

    class ContextSnapshot
    {

        public :

        bool operator==(ContextSnapshot const &) const { return true; }

    };
    class UiContext
    {


        public:



        UiContext() = default;


        ContextSnapshot snapshot() const { return ContextSnapshot(); }

    };



}
