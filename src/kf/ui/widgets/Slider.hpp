// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Range.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/internal/Adjuster.hpp"
#include "kf/ui/internal/Placement.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::internal {

template<typename T> struct SliderConfig final {
    Range<T> value_range;///< value range (slider value will clamped at this value)
    T default_value;     ///< slider value by default
    T step;              ///< slider value adjust step
    Placement placement; ///< placement of slider value
    bool init_show_value;///< show value by default
};

}// namespace kf::internal

namespace kf::ui::widgets {

struct SliderTag {};

template<typename U, typename T> struct Slider final :

    SliderTag,
    Widget<U>,
    mixin::ValueCallbacked<T>,
    mixin::Configurable<internal::SliderConfig<T>>

{
    using Config = internal::SliderConfig<T>;

    explicit constexpr Slider(const Config &config) noexcept :
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
        this->value(this->config().value_range.clamped(internal::ArithmeticAdjuster<T>::adjust(this->value(), this->config().step, event_value)));
        return true;// redraw required after adjustment
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        const auto placement = _show_value ? this->config().placement : internal::Placement::Hidden;
        render.slider(this->value(), this->config().value_range, placement);
    }

private:
    bool _show_value;
};

}// namespace kf::ui::widgets