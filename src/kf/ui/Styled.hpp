// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/Color.hpp"

namespace kf::ui {

struct Styled {

    /// @brief Widget style
    struct Style {
        Color foreground_color;///< Text color
        Color background_color;///< Background color

        /// @brief Create default style
        [[nodiscard]] static constexpr Style defaults() noexcept {
            return Style{
                .foreground_color = Color::Normal,
                .background_color = Color::Normal,
            };
        }
    };

    explicit constexpr Styled(const Style &style) noexcept : _style{style} {}

    /// @brief Get style
    [[nodiscard]] const Style &style() const noexcept {
        return _style;
    }

    /// @brief Set style
    void style(const Style &new_style) noexcept {
        _style = new_style;
    }

    /// @brief Get Foreground color
    [[nodiscard]] Color foreground() const noexcept {
        return _style.foreground_color;
    }

    /// @brief Set Foreground color
    void foreground(Color color) noexcept {
        _style.foreground_color = color;
    }

    /// @brief Get Background color
    [[nodiscard]] Color background() const noexcept {
        return _style.background_color;
    }

    /// @brief Set Background color
    void background(Color color) noexcept {
        _style.background_color = color;
    }

private:
    Style _style;
};

}// namespace kf::ui