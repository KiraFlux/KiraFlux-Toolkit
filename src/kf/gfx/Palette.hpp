// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    gfx/Palette.hpp
/// @brief   Predefined colour palette (ANSI 16 colours) for a given pixel format.

#pragma once

#include "kf/concepts.hpp"
#include "kf/pixel/Pixel.hpp"

#include "kf/mixin/CRTP.hpp"

namespace kf::gfx {

/// @brief Predefined colour palette for a given pixel format.
/// @tparam P Pixel format type (must satisfy pixel::PixelTag).
template<implements<pixel::PixelTag> P> struct Palette final {

    using PixelImpl = P;
    using ColorType = typename PixelImpl::ColorType;

    static constexpr ColorType
        black{PixelImpl::fromRgb(0x00, 0x00, 0x00)},
        dark_red{PixelImpl::fromRgb(0x80, 0x00, 0x00)},
        dark_green{PixelImpl::fromRgb(0x00, 0x80, 0x00)},
        dark_yellow{PixelImpl::fromRgb(0x80, 0x80, 0x00)},
        dark_blue{PixelImpl::fromRgb(0x00, 0x00, 0x80)},
        dark_purple{PixelImpl::fromRgb(0x80, 0x00, 0x80)},
        dark_cyan{PixelImpl::fromRgb(0x00, 0x60, 0x60)},
        light_gray{PixelImpl::fromRgb(0x80, 0x80, 0x80)},
        dark_gray{PixelImpl::fromRgb(0x30, 0x30, 0x30)},
        light_red{PixelImpl::fromRgb(0xFF, 0x20, 0x20)},
        light_green{PixelImpl::fromRgb(0x20, 0xCF, 0x20)},
        light_yellow{PixelImpl::fromRgb(0xFF, 0xFF, 0x00)},
        light_blue{PixelImpl::fromRgb(0x20, 0x20, 0xFF)},
        light_purple{PixelImpl::fromRgb(0xFF, 0x20, 0xFF)},
        light_cyan{PixelImpl::fromRgb(0x00, 0xDF, 0xCF)},
        white{PixelImpl::fromRgb(0xFF, 0xFF, 0xFF)};

private:
    static constexpr ColorType ansi_colors[16]{
        black,
        dark_red,
        dark_green,
        dark_yellow,
        dark_blue,
        dark_purple,
        dark_cyan,
        light_gray,
        dark_gray,
        light_red,
        light_green,
        light_yellow,
        light_blue,
        light_purple,
        light_cyan,
        white,
    };

public:
    /// @brief Get a colour from the ANSI colour index (0‑15).
    /// @param ansi_color_index Index of the desired colour (only lower 4 bits used).
    /// @return Colour value in the target pixel format.
    [[nodiscard]] static constexpr ColorType ansiColor(u8 ansi_color_index) noexcept {
        return ansi_colors[ansi_color_index & 0xf];
    }
};

}// namespace kf::gfx