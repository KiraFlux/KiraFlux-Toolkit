// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/internal/Adjuster.hpp"
#include "kf/ui/internal/StepAdjuster.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct SpinBoxTag {};

/// @brief Spin box for adjusting numeric values with different modes
/// @tparam T Numeric type for spin box value (must be arithmetic)
template<typename U, typename T, typename AdjusterImpl = internal::ArithmeticAdjuster<T>> struct SpinBox final : SpinBoxTag, Widget<U>, mixin::ValueCallbacked<T> {
    KF_CHECK_IMPL(AdjusterImpl, ::kf::ui::internal::AdjusterTag);
    using Value = T;///< Numeric value type

    using StepAdjuster = internal::StepAdjuster<Value>;

    explicit SpinBox(
        Value default_value = Value{},
        Value step = StepAdjuster::default_step) noexcept :
        mixin::ValueCallbacked<Value>{default_value}, _step{step} {}

    explicit SpinBox(
        typename U::PageImpl &root,
        Value default_value = Value{},
        Value step = StepAdjuster::default_step) :
        Widget<U>{root}, mixin::ValueCallbacked<Value>{default_value}, _step{step} {}

    /// @brief Toggle between value adjustment and step adjustment modes
    /// @return true (redraw required after mode change)
    [[nodiscard]] bool onClick() noexcept override {
        _is_step_setting_mode = not _is_step_setting_mode;
        return true;
    }

    /// @brief Adjust value or step based on current mode
    /// @param event_value Adjustment scale
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        if (_is_step_setting_mode) {
            StepAdjuster::adjust(_step, event_value);
        } else {
            this->value(AdjusterImpl::adjust(this->value(), _step, event_value));
        }
        return true;// redraw required after adjustment
    }

    /// @brief Render current value or step size based on mode
    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.beginAltBlock();

        if (_is_step_setting_mode) {
            render.arrow();
            render.value(_step);
        } else {
            render.value(this->value());
        }

        render.endAltBlock();
    }

private:
    Value _step;                      ///< Current step size
    bool _is_step_setting_mode{false};///< true when adjusting step size, false when adjusting value
};

}// namespace kf::ui::widgets