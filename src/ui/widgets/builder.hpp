#pragma once

#include <utility>

#include "libcore/ds/vec.hpp"
#include "libcore/shared.hpp"
#include "ui/widgets/padded.hpp"
#include "ui/widgets/stacked.hpp"
#include "ui/widgets/vflex.hpp"
#include "ui/widgets/widget.hpp"

namespace fc
{

template <typename T = Widget>
class WidgetBuilder
{
    fc::SharedPtr<Widget> _root;
    fc::SharedPtr<Widget> _current;
    fc::Vec<fc::SharedPtr<Widget>> _parents;

public:
    WidgetBuilder(fc::SharedPtr<Widget> const &widget)
        : _root(widget), _current(widget)
    {
    }

    WidgetBuilder(fc::SharedPtr<Widget> &widget)
        : _root(widget), _current(widget)
    {
    }

    WidgetBuilder(fc::SharedPtr<Widget> &&widget)
        : _root(std::move(widget)), _current(_root)
    {
    }

    template <typename... Args>
    static WidgetBuilder<T> create(Args &&...args)
    {
        auto w = fc::SharedPtr<T>::make(std::forward<Args>(args)...)
                     .template static_pointer_cast<Widget>();
        return WidgetBuilder<T>(w);
    }

    template <typename U, typename... Args>
    static WidgetBuilder<U> make(Args &&...args)
    {
        auto w = fc::SharedPtr<U>::make(std::forward<Args>(args)...)
                     .template static_pointer_cast<Widget>();
        return WidgetBuilder<U>(w);
    }

    operator fc::SharedPtr<Widget>() const
    {
        return _root;
    }

    fc::SharedPtr<Widget> end() const
    {
        return _root;
    }

    fc::SharedPtr<Widget> root() const
    {
        return _root;
    }

    fc::SharedPtr<Widget> current() const
    {
        return _current;
    }

    WidgetBuilder &up()
    {
        if (_parents.len() > 0)
        {
            _current = _parents.pop();
        }
        return *this;
    }

    auto child(auto t)
    {
        if (_current)
        {
            _current->insertChild(t);
        }
        return *this;
    }

    auto child(auto t1, auto... t)
    {
        child(t1);
        child(t...);
        return *this;
    }

    WidgetBuilder &vflex()
    {
        auto f = fc::SharedPtr<VFlex>::make().template static_pointer_cast<Widget>();
        if (_current)
        {
            _current->insertChild(f);
            _parents.push(_current);
            _current = f;
        }
        return *this;
    }

    WidgetBuilder &flex()
    {
        return vflex();
    }

    WidgetBuilder &stacked()
    {
        auto s = fc::SharedPtr<Stacked>::make().template static_pointer_cast<Widget>();
        if (_current)
        {
            _current->insertChild(s);
            _parents.push(_current);
            _current = s;
        }
        return *this;
    }

    WidgetBuilder &pad(Padded const &p)
    {
        auto pad_widget = fc::SharedPtr<LPadded>::make(p).template static_pointer_cast<Widget>();
        if (_current)
        {
            _current->insertChild(pad_widget);
            _parents.push(_current);
            _current = pad_widget;
        }
        return *this;
    }

    WidgetBuilder &pad(Padded const &p, fc::SharedPtr<Widget> child_widget)
    {
        auto pad_widget = fc::SharedPtr<LPadded>::make(p, child_widget).template static_pointer_cast<Widget>();
        if (_current)
        {
            _current->insertChild(pad_widget);
        }
        return *this;
    }

    WidgetBuilder &pad(float pad_all)
    {
        return pad(Padded(pad_all, pad_all, pad_all, pad_all));
    }

    WidgetBuilder &pad(float pad_all, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(pad_all, pad_all, pad_all, pad_all), child_widget);
    }

    WidgetBuilder &pad(float horizontal, float vertical)
    {
        return pad(Padded(horizontal, horizontal, vertical, vertical));
    }

    WidgetBuilder &pad(float horizontal, float vertical, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(horizontal, horizontal, vertical, vertical), child_widget);
    }

    WidgetBuilder &pad(float left, float right, float down, float top)
    {
        return pad(Padded(left, right, down, top));
    }

    WidgetBuilder &pad(float left, float right, float down, float top, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(left, right, down, top), child_widget);
    }

    WidgetBuilder &pl(float pad_left)
    {
        return pad(Padded(pad_left, 0, 0, 0));
    }

    WidgetBuilder &pl(float pad_left, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(pad_left, 0, 0, 0), child_widget);
    }

    WidgetBuilder &pr(float pad_right)
    {
        return pad(Padded(0, pad_right, 0, 0));
    }

    WidgetBuilder &pr(float pad_right, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(0, pad_right, 0, 0), child_widget);
    }

    WidgetBuilder &pt(float pad_top)
    {
        return pad(Padded(0, 0, 0, pad_top));
    }

    WidgetBuilder &pt(float pad_top, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(0, 0, 0, pad_top), child_widget);
    }

    WidgetBuilder &pd(float pad_down)
    {
        return pad(Padded(0, 0, pad_down, 0));
    }

    WidgetBuilder &pd(float pad_down, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(0, 0, pad_down, 0), child_widget);
    }

    WidgetBuilder &py(float pad_vertical)
    {
        return pad(Padded(0, 0, pad_vertical, pad_vertical));
    }

    WidgetBuilder &py(float pad_vertical, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(0, 0, pad_vertical, pad_vertical), child_widget);
    }

    WidgetBuilder &px(float pad_horizontal)
    {
        return pad(Padded(pad_horizontal, pad_horizontal, 0, 0));
    }

    WidgetBuilder &px(float pad_horizontal, fc::SharedPtr<Widget> child_widget)
    {
        return pad(Padded(pad_horizontal, pad_horizontal, 0, 0), child_widget);
    }
};

} // namespace fc
