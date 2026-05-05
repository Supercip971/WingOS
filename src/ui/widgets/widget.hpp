#pragma once
#include <typeinfo>

#include "gfx/canvas/canvas.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/impl/tabbed.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/shared.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "ui/context.hpp"
namespace fc
{
class Widget;



/*
 * FIXME: rework the layout rebuilding because currently it is plitted between 3 functions and can be easily simplified
 */

class State
{

public:
    Widget *parent;
};
class Widget
{

    bool _dirty;
    bool _layout_dirty;
    bool _render_dirty;
    wgfx::GRect _layout;
    wgfx::RenderCommands rendering;

public:

    template<typename T, typename... Args>
    static core::SharedPtr<Widget> $(Args&&... args)
    {
        return core::SharedPtr<T>::make(
            core::forward<Args>(args)...
        ).template static_pointer_cast<Widget>();
    }


    Widget(){};



    bool is_mutable = false;
    core::Str _key = "";

    core::Vec<core::SharedPtr<Widget>> childs;

    virtual ~Widget() {}


    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const
    {
        if(childs.len()== 1)
        {
            return childs[0]->preferred_size(constraint);
        }
        return constraint;
    }

    virtual wgfx::GRect layout(UiContext const &ctx, wgfx::GRect constraint)
    {
        // log::log$("== LAYOUT == ");
        // dump();
        log::log$("Constraint: ({}x{} - {}x{})", (long)constraint.start.x, (long)constraint.start.y, (long)constraint.end.x, (long)constraint.end.y);

        (void)ctx;

        for (auto &child : childs)
        {
            child->update_layout(ctx, constraint);
        }

        return constraint.with_size(preferred_size(constraint.size()));
    }

    virtual void render(UiContext const &ctx, wgfx::Canvas &canvas) const
    {
        (void)ctx;
        (void)canvas;
        //   for (auto &child : childs)
        //   {
        //       child->render(ctx, canvas);
        //   }
    }

    wgfx::GRect bounds() const { return _layout; }

    void setState(auto fn)
    {
        fn();
        _dirty = true;
    }

    auto name() const { return typeid(*this).name(); };

    virtual core::Str info() const { return ""; };
    void dump(int depth = 0) const
    {

        log::log$("{} - widget[{}]: {} [{}] ({}x{} - {}x{})", fmt::Tabbed(depth), _key, name(), info(),
                  (long)_layout.start.x, (long)_layout.start.y, (long)_layout.end.x, (long)_layout.end.y);
        for (auto child : childs)
        {
            child->dump(depth + 1);
        }
    }

    bool canUpdate(const Widget &rhs) const
    {
        return typeid(*this) == typeid(rhs) && this->_key == rhs._key;
    }

    virtual core::SharedPtr<Widget> build(UiContext const &) { return {}; }
    virtual core::Vec<core::SharedPtr<Widget>> build_childs(UiContext const &v)
    {
        core::Vec<core::SharedPtr<Widget>> result = {};

        auto v2 = build(v);
        if (v2)
        {
            result.push(v2);
        }
        return result;
    };

    virtual void dispatch_relayout(UiContext const &ctx, wgfx::GRect constraint)
    {
        for (auto child : childs)
        {
            child->relayout(ctx, constraint);
        }
    }

    virtual void relayout(UiContext const &ctx, wgfx::GRect constraint)
    {
        log::log$("relayout: {} - {} {} {} {}", name(), (long)constraint.start.x, (long)constraint.start.y, (long)constraint.end.x, (long)constraint.end.y);
        // manage how to layout childs
        auto nlayout = layout(ctx, constraint);
        //  log::log$("relayout: {} - {} {} {} {}", name(), (long)nlayout.start.x, (long)nlayout.start.y, (long)nlayout.end.x, (long)nlayout.end.y);

        if (_layout != nlayout)
        {
            _layout = nlayout;
            _render_dirty = true;

        }

        _layout_dirty = false;
    }

    virtual void rerender(UiContext const &ctx, wgfx::Canvas &canvas)
    {
        if (_render_dirty)
        {
            auto ncanvas = canvas.record();

            this->render(ctx, ncanvas);

            rendering = ncanvas.stopRecord();

            log::log$("rendering: {}", rendering.len());
            canvas.recordApply(rendering, _layout);
            _render_dirty = false;
        }
        else
        {
            log::log$("rerendering: {}", rendering.len());

            canvas.recordApply(rendering, _layout);
        }
        for (auto &child : childs)
        {
            child->rerender(ctx, canvas);
        }
    }

    virtual void update_layout(UiContext const &ctx, wgfx::GRect constraint)
    {
        if (_layout_dirty)
        {
            relayout(ctx, constraint);
        }
    }

    virtual void update_dirty(UiContext const &ctx)
    {
        for (auto &child : childs)
        {
            child->update_dirty(ctx);
        }

        if (_dirty)
        {
            rebuild(ctx);
        }
    }


    virtual bool transferTo(Widget & other)  {

        (void)other;
        other._layout = _layout;
        other.rendering = rendering;
        other.childs = core::move(childs);
        return false;
    }

    virtual void update(UiContext const &ctx, core::SharedPtr<Widget> t)
    {
        (void)ctx;
        *this = *t;
        rebuild(ctx);
    }
    virtual void mount(UiContext const &ctx)
    {
        rebuild(ctx);
    }

    virtual void unmount()
    {
    }
    virtual void rebuild(UiContext const &ctx)
    {
        _dirty = false;
        _layout_dirty = true;
        _render_dirty = true;
        auto rebuild_widget = (build_childs(ctx)); // rebuild the new widget

        log::log$("==== REBUILDING ====");
        dump();
        // leaf
        if (rebuild_widget.len() == 1 && childs.len() == 1)
        {
            if (childs[0]->canUpdate(*rebuild_widget[0]))
            {
                log::log$("transferTo: {}", childs[0]->name());
                childs[0]->transferTo(*rebuild_widget[0]);

                childs[0] = rebuild_widget[0];
                childs[0]->rebuild(ctx);
            }
            else
            {
           //     childs[0]->transferTo(*rebuild_widget[0]);

                childs[0] = rebuild_widget[0];
                childs[0]->mount(ctx);
            }

            return;
        }
        else if (rebuild_widget.len() == 0 && childs.len() == 0)
        {

            return;
        }

        core::Vec<core::SharedPtr<Widget>> new_childs = {};

        size_t old_idx = 0;

        // iterate through every widget:
        // - if we have a 'new' widget with a key:
        //  - see if we already had it
        //  - if not create it
        // - if it don't have a key, try to conserve ordering in mind.
        //
        for (size_t i = 0; i < rebuild_widget.len(); i++)
        {
            core::SharedPtr<Widget> matched_elm = {};

            if (!rebuild_widget[i]->_key.is_empty())
            {
                for (size_t j = 0; j < childs.len(); j++)
                {
                    if (childs[j]->canUpdate(*rebuild_widget[i]))
                    {

                        matched_elm = childs[j];
                        childs.pop(j);
                        break;
                    }
                }
            }
            else
            {
                if (old_idx < childs.len() &&
                    childs[old_idx]->canUpdate(*rebuild_widget[i]))
                {
                    matched_elm = childs[old_idx];

                }
                old_idx++;
            }

            if (matched_elm)
            {
                log::log$("transferTo: {} -> {}", matched_elm->name(), rebuild_widget[i]->name());
                matched_elm->transferTo(*rebuild_widget[i]);
                matched_elm = rebuild_widget[i];
                matched_elm->rebuild(ctx);
                new_childs.push(matched_elm);
            }
            else
            {

                auto new_elm = rebuild_widget[i];
                new_elm->mount(ctx);
                new_childs.push(new_elm);
            }
        }

        for (auto &v : childs)
        {
            if (v->_key)
            {
                v->unmount();
            }
        }

        childs = core::move(new_childs);
    }
};



template <typename T, typename... Args>
core::SharedPtr<Widget> mount(UiContext const &ctx, Args &&...args)
{
    return T::construct(ctx, core::forward<Args>(args)...);
}

} // namespace fc
