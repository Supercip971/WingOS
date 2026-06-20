#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/text/font.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/path.hpp"
#include "libcore/shared.hpp"
namespace fc
{
class FontsRepo
{

    core::UMap<core::WStr, core::SharedPtr<wgfx::Font>> _fonts = {};

public:
    using AppRessource = core::SharedPtr<wgfx::Font>;
    core::Result<void> load(core::WStr const &name, core::Str path, size_t size = 96)
    {

        fmt::log$("loading font({}): {}", name.view(), path);
        auto fpath = core::finalizePath(path);
        auto t = wgfx::Typeface::from_file(fpath.view()).copied();

        _fonts.insert(
            core::WStr::copy(name.view()),
            core::SharedPtr<wgfx::Font>::make(try$(wgfx::Font::load_font(t, size))));
        return {};
    }
    core::SharedPtr<wgfx::Font> &find(core::Str const &name)
    {
        return _fonts[name];
    }
    core::SharedPtr<wgfx::Font> const &find(core::Str const &name) const
    {
        return _fonts[name];
    }
    core::UMap<core::WStr, core::SharedPtr<wgfx::Font>> &raw()
    {
        return _fonts;
    }
    core::UMap<core::WStr, core::SharedPtr<wgfx::Font>> const &raw() const
    {
        return _fonts;
    }
    static FontsRepo &the();
};

} // namespace fc
