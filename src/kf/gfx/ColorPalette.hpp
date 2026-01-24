// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/pixel_traits.hpp"
#include "kf/core/PixelFormat.hpp"


namespace kf {

template<PixelFormat F> struct ColorPalette {
    using traits = pixel_traits<F>;

    using ColorType = typename traits::ColorType;

    enum class Ansi : u8 {
        Black = 0x0,
        Red = 0x1,
        Green = 0x2,
        Yellow = 0x3,
        Blue = 0x4,
        Purple = 0x5,
        Cyan = 0x6,
        White = 0x7,
        BlackBright = 0x8,
        RedBright = 0x9,
        GreenBright = 0xA,
        YellowBright = 0xB,
        BlueBright = 0xC,
        PurpleBright = 0xD,
        CyanBright = 0xE,
        WhiteBright = 0xF,
    };

private:

    static constexpr ColorType ansi_colors[16]{
        // standard
        traits::fromRgb(0x00, 0x00, 0x00), // black
        traits::fromRgb(0x80, 0x00, 0x00), // red
        traits::fromRgb(0x00, 0x80, 0x00), // green
        traits::fromRgb(0x80, 0x80, 0x00), // yellow
        traits::fromRgb(0x00, 0x00, 0x80), // blue
        traits::fromRgb(0x80, 0x00, 0x80), // purple
        traits::fromRgb(0x00, 0x70, 0x80), // cyan
        traits::fromRgb(0x80, 0x80, 0x80), // white
        // intense
        traits::fromRgb(0x60, 0x60, 0x60), // bright black
        traits::fromRgb(0xFF, 0x20, 0x20), // bright red
        traits::fromRgb(0x20, 0xCF, 0x20), // bright green
        traits::fromRgb(0xFF, 0xFF, 0x00), // bright yellow
        traits::fromRgb(0x20, 0x20, 0xFF), // bright blue
        traits::fromRgb(0xFF, 0x20, 0xFF), // bright purple
        traits::fromRgb(0x00, 0xDF, 0xCF), // bright cyan
        traits::fromRgb(0xFF, 0xFF, 0xFF), // bright white
    };

public:

    static constexpr ColorType getAnsiColor(Ansi ansi_color_index) {
        return ansi_colors[static_cast<u8>(ansi_color_index) & 0xf];
    }
};

}// namespace kf
