// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Callbacked.hpp"

#include "kf/ui/internal/StepAdjuster.hpp"
#include "kf/ui/internal/ValueAdjuster.hpp"
#include "kf/ui/internal/ValueLimits.hpp"
#include "kf/ui/internal/ValuePlacement.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct SliderTag {};

template<typename U, typename T> struct Slider final : SliderTag, Widget<U>, mixin::Callbacked<T> {
    using Value = T;
    using StepAdjuster = internal::StepAdjuster<Value>;
    using ValueAdjuster = internal::ValueAdjuster<Value, internal::StepMode::Arithmetic>;
    using ValueLimits = internal::ValueLimits<Value>;

    explicit Slider(
        Value default_value = Value{},
        Value step = StepAdjuster::default_step) noexcept :
        _value{default_value}, _step{step} {}

    explicit Slider(
        typename U::PageImpl &root,
        Value default_value = Value{},
        Value step = StepAdjuster::default_step) :
        Widget<U>{root}, _value{default_value}, _step{step} {}

    void value(Value new_value) noexcept {
        new_value = kf::clamp(new_value, min_value, max_value);
        if (_value != new_value) {
            _value = new_value;
            mixin::Callbacked<Value>::invoke(_value);
        }
    }

    [[nodiscard]] Value value() const noexcept { return _value; }

    /// @brief Toggle slider numeric value display
    [[nodiscard]] bool onClick() noexcept override {
        show_value = not show_value;
        return true;// redraw required after mode change
    }

    /// @brief Adjust value
    /// @param event_value adjust change scale
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        ValueAdjuster::adjust(_value, _step, event_value);
        _value = kf::clamp(_value, min_value, max_value);
        mixin::Callbacked<Value>::invoke(_value);
        return true;// redraw required after adjustment
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.slider(_value, min_value, max_value, show_value ? placement : internal::ValuePlacement::Hidden);
    }

    Value min_value{ValueLimits::value_min};
    Value max_value{ValueLimits::value_max};
    bool show_value{false};
    internal::ValuePlacement placement{internal::ValuePlacement::Inside};

private:
    Value _value;
    Value _step;
};

}// namespace kf::ui::widgets