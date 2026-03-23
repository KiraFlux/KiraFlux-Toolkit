// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/internal/StepAdjuster.hpp"
#include "kf/ui/internal/ValueAdjuster.hpp"
#include "kf/ui/internal/ValueLimits.hpp"
#include "kf/ui/internal/ValuePlacement.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct SliderTag {};

template<typename U, typename T> struct Slider final : SliderTag, Widget<U>, mixin::ValueCallbacked<T> {
    using Value = T;
    using StepAdjuster = internal::StepAdjuster<Value>;
    using ValueAdjuster = internal::ValueAdjuster<Value, internal::StepMode::Arithmetic>;
    using ValueLimits = internal::ValueLimits<Value>;

    explicit Slider(
        Value default_value = Value{},
        Value step = StepAdjuster::default_step) noexcept :
        mixin::ValueCallbacked<T>{default_value}, _step{step} {}

    explicit Slider(
        typename U::PageImpl &root,
        Value default_value = Value{},
        Value step = StepAdjuster::default_step) :
        Widget<U>{root}, mixin::ValueCallbacked<T>{default_value}, _step{step} {}

    /// @brief Toggle slider numeric value display
    [[nodiscard]] bool onClick() noexcept override {
        show_value = not show_value;
        return true;// redraw required after mode change
    }

    /// @brief Adjust value
    /// @param event_value adjust change scale
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        this->value(kf::clamp(ValueAdjuster::adjust(this->value(), _step, event_value), min_value, max_value));
        return true;// redraw required after adjustment
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.slider(this->value(), min_value, max_value, show_value ? placement : internal::ValuePlacement::Hidden);
    }

    Value min_value{ValueLimits::value_min};
    Value max_value{ValueLimits::value_max};
    internal::ValuePlacement placement{internal::ValuePlacement::Inside};
    bool show_value{false};

private:
    Value _step;
};

}// namespace kf::ui::widgets