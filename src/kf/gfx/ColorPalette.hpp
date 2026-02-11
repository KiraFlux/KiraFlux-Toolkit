// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/attributes.hpp"
#include "kf/pixel/PixelFormat.hpp"

namespace kf {// NOLINT(*-concat-nested-namespaces)
namespace gfx {

template<typename F> struct ColorPalette final {
    using PixelFormat = F;

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
    static constexpr typename PixelFormat::ColorType ansi_colors[16]{
        // standard
        PixelFormat::fromRgb(0x00, 0x00, 0x00),// black
        PixelFormat::fromRgb(0x80, 0x00, 0x00),// red
        PixelFormat::fromRgb(0x00, 0x80, 0x00),// green
        PixelFormat::fromRgb(0x80, 0x80, 0x00),// yellow
        PixelFormat::fromRgb(0x00, 0x00, 0x80),// blue
        PixelFormat::fromRgb(0x80, 0x00, 0x80),// purple
        PixelFormat::fromRgb(0x00, 0x60, 0x60),// cyan
        PixelFormat::fromRgb(0x80, 0x80, 0x80),// white
        // intense
        PixelFormat::fromRgb(0x30, 0x30, 0x30),// bright black
        PixelFormat::fromRgb(0xFF, 0x20, 0x20),// bright red
        PixelFormat::fromRgb(0x20, 0xCF, 0x20),// bright green
        PixelFormat::fromRgb(0xFF, 0xFF, 0x00),// bright yellow
        PixelFormat::fromRgb(0x20, 0x20, 0xFF),// bright blue
        PixelFormat::fromRgb(0xFF, 0x20, 0xFF),// bright purple
        PixelFormat::fromRgb(0x00, 0xDF, 0xCF),// bright cyan
        PixelFormat::fromRgb(0xFF, 0xFF, 0xFF),// bright white
    };

public:
    kf_nodiscard static constexpr typename PixelFormat::ColorType getAnsiColor(Ansi ansi_color_index) noexcept {
        return ansi_colors[static_cast<u8>(ansi_color_index) & 0xf];
    }
};

}// namespace gfx
}// namespace kf