// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/Color.hpp"
#include "kf/ui/Style.hpp"

namespace kf::mixin {

struct Styled {

    explicit constexpr Styled(ui::Style const &style) noexcept :
        _style{style} {}

    /// @brief Get style
    [[nodiscard]] constexpr ui::Style const &style() const noexcept {
        return _style;
    }

    /// @brief Set style
    constexpr void style(ui::Style const &new_style) noexcept {
        _style = new_style;
    }

    /// @brief Get Foreground color
    [[nodiscard]] constexpr ui::Color foreground() const noexcept {
        return _style.foreground_color;
    }

    /// @brief Set Foreground color
    constexpr void foreground(ui::Color color) noexcept {
        _style.foreground_color = color;
    }

    /// @brief Get Background color
    [[nodiscard]] constexpr ui::Color background() const noexcept {
        return _style.background_color;
    }

    /// @brief Set Background color
    constexpr void background(ui::Color color) noexcept {
        _style.background_color = color;
    }

private:
    ui::Style _style;
};

}// namespace kf::mixin