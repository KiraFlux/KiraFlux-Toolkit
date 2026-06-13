// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Range.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/Placement.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::internal {

template<typename T> struct SliderConfig final {
    Range<T> value_range;   ///< value range (slider value will clamped at this value)
    T default_value;        ///< slider value by default
    T step;                 ///< slider value adjust step
    ui::Placement placement;///< placement of slider value
    bool init_show_value;   ///< show value by default
};

}// namespace kf::internal

namespace kf::ui::widgets {

struct SliderTag {};

/// @tparam U UI Traits Type
/// @tparam T Type of slider Value
template<typename U, typename T> struct Slider :

    SliderTag,
    U::Widget,
    mixin::ValueCallbacked<T>,
    mixin::Configurable<internal::SliderConfig<T>>

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    using Config = internal::SliderConfig<T>;

    explicit constexpr Slider(const Config &config, typename U::Widget::Style style = U::Widget::Style::defaults()) noexcept :
        U::Widget{style},
        mixin::ValueCallbacked<T>{config.value_range.clamped(config.default_value)},
        mixin::Configurable<Config>{config}, _show_value{config.init_show_value} {}

    /// @brief Toggle slider numeric value display
    [[nodiscard]] bool onClick() noexcept override {
        _show_value = not _show_value;
        return true;// redraw required after mode change
    }

    /// @brief Adjust value
    /// @param event_value adjust change scale
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        this->value(this->config().value_range.clamped(U::template ArithmeticAdjuster<T>::adjust(this->value(), this->config().step, event_value)));
        return true;// redraw required after adjustment
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        const auto placement = _show_value ? this->config().placement : Placement::Hidden;
        render.slider(this->value(), this->config().value_range, placement);
    }

private:
    bool _show_value;
};

}// namespace kf::ui::widgets