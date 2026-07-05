#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/text/font.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/path.hpp"
#include "libcore/result.hpp"
#include "libcore/shared.hpp"
#include "libcore/str.hpp"

namespace fc
{
class FontsRepo
{

    fc::UMap<fc::WStr, fc::SharedPtr<wgfx::Font>> _fonts = {};

public:
    using AppRessource = fc::SharedPtr<wgfx::Font>;

    fc::Result<void> load(fc::WStr const &name, fc::Str path, size_t size = 96)
    {

        fmt::log$("loading font({}): {}", name.view(), path);
        auto fpath = fc::finalizePath(path);
        auto t = wgfx::Typeface::from_file(fpath.view()).copied();

        _fonts.insert(
            fc::WStr::copy(name.view()),
            fc::SharedPtr<wgfx::Font>::make(try$(wgfx::Font::load_font(t, size))));
        return {};
    }

    fc::SharedPtr<wgfx::Font> &find(fc::Str const &name)
    {
        return _fonts[name];
    }

    fc::SharedPtr<wgfx::Font> const &find(fc::Str const &name) const
    {
        return _fonts[name];
    }

    fc::UMap<fc::WStr, fc::SharedPtr<wgfx::Font>> &raw()
    {
        return _fonts;
    }

    fc::UMap<fc::WStr, fc::SharedPtr<wgfx::Font>> const &raw() const
    {
        return _fonts;
    }

    static FontsRepo &the();
};

} // namespace fc
