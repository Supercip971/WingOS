#pragma once
#include <typeinfo>

#include "gfx/canvas/canvas.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/event/event.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/impl/tabbed.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/optional.hpp"
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

    bool _dirty = true;
    bool _layout_dirty = true;
    bool _render_dirty = true;
    wgfx::GRect _old_render_layout = {};
    wgfx::GRect _layout{};
    wgfx::RenderCommands rendering = {};
    float dirty_dep_width = 0.0f;

public:
    float dirty_dependence_around() const { return dirty_dep_width; }
    void dirty_depenced_around(float val) { dirty_dep_width = val; }
    auto name() const { return typeid(*this).name(); };
    virtual bool acquireEvent(wgfx::UEvent ev)
    {
        if (ev.kind == wgfx::UEvent::Kind::MOUSE_MOVE || ev.kind == wgfx::UEvent::Kind::MOUSE_CLICK)
        {
            if (this->_layout.size() != wgfx::Vec2(0, 0) &&
                !this->_layout.contains(ev.at))
            {
                return false;
            }
        }
        for (auto &child : childs)
        {
            if (child->acquireEvent(ev))
            {
                return true;
            }
        }
        return false;
    }
    template <typename T, typename... Args>
    static core::SharedPtr<Widget> $(Args &&...args)
    {
        return core::SharedPtr<T>::make(
                   core::forward<Args>(args)...)
            .template static_pointer_cast<Widget>();
    }

    Widget() {};

    bool is_mutable = false;
    core::Str _key = "";

    core::Vec<core::SharedPtr<Widget>> childs;

    virtual ~Widget() {}

    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const
    {
        if (childs.len() == 1)
        {
            return childs[0]->preferred_size(constraint);
        }
        return constraint;
    }

    virtual wgfx::GRect layout(UiContext const &ctx, wgfx::GRect constraint)
    {
        // fmt::log$("== LAYOUT == ");
        // dump();
        fmt::log$("Constraint: ({}x{} - {}x{})", (long)constraint.start.x, (long)constraint.start.y, (long)constraint.end.x, (long)constraint.end.y);

        (void)ctx;

        auto s = constraint.with_size(preferred_size(constraint.size()));
        for (auto &child : childs)
        {
            child->update_layout(ctx, s);
        }

        return s;
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

    virtual core::Str info() const { return ""; };
    void dump(int depth = 0) const
    {

        fmt::log$("{} - widget[{}]: {} [{}] ({}x{} - {}x{})", fmt::Tabbed(depth), _key, name(), info(),
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

    virtual void relayout(UiContext const &ctx, wgfx::GRect constraint)
    {
        fmt::log$("relayout: {} - {} {} {} {}", name(), (long)constraint.start.x, (long)constraint.start.y, (long)constraint.end.x, (long)constraint.end.y);
        // manage how to layout childs
        auto nlayout = layout(ctx, constraint);
        //  fmt::log$("relayout: {} - {} {} {} {}", name(), (long)nlayout.start.x, (long)nlayout.start.y, (long)nlayout.end.x, (long)nlayout.end.y);

        if (_layout != nlayout)
        {
            _layout = nlayout;
            _render_dirty = true;
        }

        _layout_dirty = false;
    }

    virtual core::Optional<wgfx::GRect> render_dirty_rect()
    {
        if (_render_dirty)
        {
            auto m = _old_render_layout.merge(_layout);
            if (dirty_dep_width != 0)
            {
                m.start.x -= dirty_dep_width;
                m.end.x += dirty_dep_width;
                m.start.y -= dirty_dep_width;
                m.end.y += dirty_dep_width;
            }
            return m;
        }

        if (childs.len() == 0)
        {
            return {};
        }

        core::Optional<wgfx::GRect> f{core::novalue};
        for (auto &child : childs)
        {
            auto rect = child->render_dirty_rect();

            if (!rect.has_value())
            {
                continue;
            }

            if (f.has_value())
            {
                f = f->merge(rect.value());
            }
            else
            {
                f = rect;
            }
        }

        if (f.has_value())
        {

            f->start.x -= dirty_dep_width;
            f->end.x += dirty_dep_width;
            f->start.y -= dirty_dep_width;
            f->end.y += dirty_dep_width;
        }
        return f;
    }
    virtual bool render_dirty(UiContext const &ctx, wgfx::Canvas &canvas)
    {

        if (_render_dirty)
        {

            auto ncanvas = canvas.record();
            this->render(ctx, ncanvas);

            if (ctx.enable_debug_layout)
            {
                ncanvas.drawRect(this->_layout, wgfx::Painter::stroked(wgfx::RED, 2.f));
            }

            rendering = ncanvas.stopRecord();
            canvas.recordApply(rendering, _layout);

            _old_render_layout = _layout;
            for (auto &child : childs)
            {
                child->render_dirty(ctx, canvas);
            }

            _render_dirty = false;
            return true;
        }

        core::Optional<wgfx::GRect> dirty_rect = this->render_dirty_rect();

        if (!dirty_rect.has_value())
        {
            return false;
        }

        canvas.recordApply(rendering, (dirty_rect.value()));
        for (auto &child : childs)
        {
            auto r = child->render_dirty_rect();

            // child is not dirty but inside a dirty rect:
            // for example a stacked widget, that has 2 widget on top of each other with one transparent
            // meaning that we need to rerender the one behind even if it is not dirty
            if (!r.has_value())
            {

                if (child->_old_render_layout.does_intersect(dirty_rect.value()))
                {
                    canvas.recordApply(child->rendering,
                                       child->_old_render_layout.intersect(dirty_rect.value()));
                }
                continue;
            }

            child->render_dirty(ctx, canvas);
        }

        return false;
    }

    virtual void rerender(UiContext const &ctx, wgfx::Canvas &canvas)
    {
        if (_render_dirty)
        {
            auto ncanvas = canvas.record();

            this->render(ctx, ncanvas);

            rendering = ncanvas.stopRecord();

            _old_render_layout = _layout;
            fmt::log$("rendering: {}", rendering.len());
            canvas.recordApply(rendering, _layout);
            _render_dirty = false;
        }
        else
        {

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

    virtual bool update_dirty(UiContext const &ctx)

    {
        bool d = false;
        for (auto &child : childs)
        {
            d |= child->update_dirty(ctx);
        }

        if (d)
        {
            this->_layout_dirty = true;
        }
        if (_dirty)
        {
            d = true;
            rebuild(ctx);
        }

        return d;
    }

    virtual bool transferTo(Widget &other)
    {

        (void)other;
        other._layout = _layout;
        other._old_render_layout = _old_render_layout;
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

        fmt::log$("==== REBUILDING ====");
        dump();
        // leaf
        if (rebuild_widget.len() == 1 && childs.len() == 1)
        {
            if (childs[0]->canUpdate(*rebuild_widget[0]))
            {
                fmt::log$("transferTo: {}", childs[0]->name());
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
                fmt::log$("transferTo: {} -> {}", matched_elm->name(), rebuild_widget[i]->name());
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
