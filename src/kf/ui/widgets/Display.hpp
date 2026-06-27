// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/Request.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::ui::widgets {

struct DisplayTag {};

/// @brief Display widget for showing read-only values
/// @tparam U UI Traits Type
/// @tparam T Type of value to display
template<typename U, typename T> struct Display :

    DisplayTag,
    U::Widget

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    constexpr Display() noexcept :
        U::Widget{Style::defaults()}, _value{} {}

    explicit constexpr Display(const T &value, Style style = Style::defaults()) noexcept :
        U::Widget{style}, _value{value} {}

    /// @brief Get the current displayed value
    [[nodiscard]] constexpr const T &value() const noexcept {
        return _value;
    }

    /// @brief Update the displayed value
    void value(const T &new_value) noexcept {
        _value = new_value;
    }

    void doRender(typename U::RendererImpl &render) const noexcept override {
        render.value(_value);
    }

private:
    T _value;///< Stored copy of the value
};

}// namespace kf::ui::widgets