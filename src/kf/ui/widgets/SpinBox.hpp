// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/internal/Adjuster.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui {

namespace internal {

template<typename T> struct SpinBoxConfig final : mixin::NonCopyable {
    T default_step;///< value adjust step default value
    T step_adjust; ///< adjust step of step
};

}// namespace internal

namespace widgets {

struct SpinBoxTag {};

/// @brief Spin box for adjusting numeric values with different modes
/// @tparam T Numeric type for spin box value (must be arithmetic)
template<typename U, typename T, typename AdjusterImpl = internal::ArithmeticAdjuster<T>>
struct SpinBox final : SpinBoxTag, Widget<U>, mixin::ValueCallbacked<T>, mixin::Configurable<internal::SpinBoxConfig<T>> {
    KF_CHECK_IMPL(AdjusterImpl, ::kf::ui::internal::AdjusterTag);
    using Config = internal::SpinBoxConfig<T>;

    constexpr explicit SpinBox(const Config &config, T default_value = T{}) noexcept :
        mixin::ValueCallbacked<T>{default_value}, mixin::Configurable<Config>{config}, _step(config.default_step) {}

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
            _step = internal::GeometricAdjuster<T>::adjust(_step, this->config().step_adjust, event_value);
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
    T _step;                          ///< Current step size
    bool _is_step_setting_mode{false};///< true when adjusting step size, false when adjusting value
};

}// namespace widgets

}// namespace kf::ui