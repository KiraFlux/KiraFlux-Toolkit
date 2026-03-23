// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/internal/StepAdjuster.hpp"
#include "kf/ui/internal/StepMode.hpp"
#include "kf/ui/internal/ValueAdjuster.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct SpinBoxTag {};

/// @brief Spin box for adjusting numeric values with different modes
/// @tparam T Numeric type for spin box value (must be arithmetic)
template<typename U, typename T, internal::StepMode M> struct SpinBox final : SpinBoxTag, Widget<U>, mixin::ValueCallbacked<T> {
    using Value = T;                   ///< Numeric value type
    static constexpr auto step_mode{M};///< Specialization step mode

    using StepAdjuster = internal::StepAdjuster<Value>;
    using ValueAdjuster = internal::ValueAdjuster<Value, step_mode>;

    explicit SpinBox(
        Value default_value = Value{},
        Value step = StepAdjuster::default_step) noexcept :
        mixin::ValueCallbacked<T>{default_value}, _step{step} {}

    explicit SpinBox(
        typename U::PageImpl &root,
        Value default_value = Value{},
        Value step = StepAdjuster::default_step) :
        Widget<U>{root}, mixin::ValueCallbacked<T>{default_value}, _step{step} {}

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
            auto v{this->value()};
            ValueAdjuster::adjust(v, _step, event_value);
            this->value(v);
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