// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/pixel/Pixel.hpp"

namespace kf::gfx {

template<typename P> struct Palette final {
    using PixelImpl = P;
    using ColorType = typename PixelImpl::ColorType;

    static constexpr ColorType
        black{PixelImpl::fromRgb(0x00, 0x00, 0x00)},
        red{PixelImpl::fromRgb(0x80, 0x00, 0x00)},
        green{PixelImpl::fromRgb(0x00, 0x80, 0x00)},
        yellow{PixelImpl::fromRgb(0x80, 0x80, 0x00)},
        blue{PixelImpl::fromRgb(0x00, 0x00, 0x80)},
        purple{PixelImpl::fromRgb(0x80, 0x00, 0x80)},
        cyan{PixelImpl::fromRgb(0x00, 0x60, 0x60)},
        white{PixelImpl::fromRgb(0x80, 0x80, 0x80)},
        bright_black{PixelImpl::fromRgb(0x30, 0x30, 0x30)},
        bright_red{PixelImpl::fromRgb(0xFF, 0x20, 0x20)},
        bright_green{PixelImpl::fromRgb(0x20, 0xCF, 0x20)},
        bright_yellow{PixelImpl::fromRgb(0xFF, 0xFF, 0x00)},
        bright_blue{PixelImpl::fromRgb(0x20, 0x20, 0xFF)},
        bright_purple{PixelImpl::fromRgb(0xFF, 0x20, 0xFF)},
        bright_cyan{PixelImpl::fromRgb(0x00, 0xDF, 0xCF)},
        bright_white{PixelImpl::fromRgb(0xFF, 0xFF, 0xFF)};

private:
    static constexpr ColorType ansi_colors[16]{
        black,
        red,
        green,
        yellow,
        blue,
        purple,
        cyan,
        white,
        bright_black,
        bright_red,
        bright_green,
        bright_yellow,
        bright_blue,
        bright_purple,
        bright_cyan,
        bright_white,
    };

public:
    [[nodiscard]] static constexpr ColorType ansiColor(u8 ansi_color_index) noexcept {
        return ansi_colors[ansi_color_index & 0xf];
    }
};

}// namespace kf::gfx