
#pragma once
#include "gfx/backend.hpp"
#include "gfx/canvas/canvas.hpp"
#include "gfx/event/event.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/shared.hpp"
#include "libcore/type-utils.hpp"
#include "ui/context.hpp"

namespace wgfx
{

class PlatformWindow : public core::NoCopy
{
public:
    uint64_t id;
    bool attached = false;

    BackendsKinds backend_kind;

    virtual core::Result<void> attach() { return {}; };

    virtual float dpi() { return 1.f; }

    virtual ~PlatformWindow() {};

    virtual void destroy()
    {
        fmt::warn$("Window::destroy backend not implemented");
    }

    virtual size_t width()
    {
        fmt::warn$("Window::width backend not implemented");
        return 0;
    }

    virtual size_t height()
    {
        fmt::warn$("Window::height backend not implemented");
        return 0;
    }

    virtual Canvas *create_frame()
    {
        fmt::warn$("Canvas unable to be acquired");
        return nullptr;
    }

    virtual void end_frame(Canvas *frame)
    {
        (void)frame;
        fmt::warn$("Canvas was unable to be swapped");
    }
    virtual void set_width(size_t width [[maybe_unused]])
    {
        fmt::warn$("Window::set_width not implemented");
    }

    virtual void set_height(size_t height [[maybe_unused]])
    {
        fmt::warn$("Window::set_height not implemented");
    }


    virtual wgfx::UEvent query_event()
    {
        fmt::warn$("Window::query_event not implemented");
        return {};
    }

    static core::Result<core::SharedPtr<PlatformWindow>> create_native(BackendsKinds preferred_backend = BackendsKinds::BACKEND_KIND_RASTER);
};

} // namespace wgfx
