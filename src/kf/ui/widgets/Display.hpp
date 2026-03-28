// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct DisplayTag {};

/// @brief Display widget for showing read-only values
/// @tparam T Type of value to display
template<typename U, typename T> struct Display final : DisplayTag, Widget<U> {

    constexpr explicit Display(T value) noexcept : _value{value} {}

    /// @brief Update the displayed value
    void value(T new_value) noexcept { _value = new_value; }

    /// @brief Get the current displayed value
    [[nodiscard]] T value() const noexcept { return _value; }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.value(_value);
    }

private:
    T _value;///< Stored copy of the value
};

}// namespace kf::ui::widgets