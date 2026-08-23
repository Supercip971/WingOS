#pragma once

#include "widget.hpp"

namespace fc
{

template <typename State>
class Statefull : public Widget, public State
{

public:
    Statefull() : Widget(), State() {}

    bool transferTo(Widget &other) override
    {
        static_cast<Statefull<State> &>(other).State::operator=(static_cast<const State &>(*this));
        other.childs = std::move(childs);
        return true;
    }

    virtual ~Statefull() override = default;
};
} // namespace fc
