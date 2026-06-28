#pragma once

#include "gfx/color.hpp"

namespace fc
{
class Theme
{
public:
    virtual ~Theme() = default;

    struct ConstainerStyle
    {
        wgfx::CompositeColor container_bg;
        wgfx::CompositeColor container_border;

        float border_size;
        float elevation;
        float radius;
    } container;

    struct TextStyle
    {
        wgfx::CompositeColor text_color;
    } text;

    static constexpr Theme wingos()
    {
        Theme theme;
        theme.container = {
            .container_bg = wgfx::CONTAINER_FILL,
            .container_border = wgfx::CONTAINER_BORDER,
            .border_size = 2.f,
            .elevation = 0.f,
            .radius = 0.015f};
        theme.text = {
            .text_color = wgfx::SLATE_WHITE};
        return theme;
    }
};
}; // namespace fc
