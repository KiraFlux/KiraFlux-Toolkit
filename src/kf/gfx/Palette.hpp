// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/pixel/Pixel.hpp"

namespace kf::gfx {

template<typename P> struct Palette final {
    using PixelImpl = P;
    using ColorType = typename PixelImpl::ColorType;

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
        PixelImpl::fromRgb(0x00, 0x00, 0x00),// black
        PixelImpl::fromRgb(0x80, 0x00, 0x00),// red
        PixelImpl::fromRgb(0x00, 0x80, 0x00),// green
        PixelImpl::fromRgb(0x80, 0x80, 0x00),// yellow
        PixelImpl::fromRgb(0x00, 0x00, 0x80),// blue
        PixelImpl::fromRgb(0x80, 0x00, 0x80),// purple
        PixelImpl::fromRgb(0x00, 0x60, 0x60),// cyan
        PixelImpl::fromRgb(0x80, 0x80, 0x80),// white
        // intense
        PixelImpl::fromRgb(0x30, 0x30, 0x30),// bright black
        PixelImpl::fromRgb(0xFF, 0x20, 0x20),// bright red
        PixelImpl::fromRgb(0x20, 0xCF, 0x20),// bright green
        PixelImpl::fromRgb(0xFF, 0xFF, 0x00),// bright yellow
        PixelImpl::fromRgb(0x20, 0x20, 0xFF),// bright blue
        PixelImpl::fromRgb(0xFF, 0x20, 0xFF),// bright purple
        PixelImpl::fromRgb(0x00, 0xDF, 0xCF),// bright cyan
        PixelImpl::fromRgb(0xFF, 0xFF, 0xFF),// bright white
    };

public:
    [[nodiscard]] static constexpr ColorType ansiColor(Ansi ansi_color_index) noexcept {
        return ansi_colors[static_cast<u8>(ansi_color_index) & 0xf];
    }
};

}// namespace kf::gfx