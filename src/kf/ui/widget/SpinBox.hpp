// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/concepts.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/Block.hpp"
#include "kf/ui/Color.hpp"
#include "kf/ui/Decoration.hpp"
#include "kf/ui/Request.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::internal {

template<arithmetic T> struct SpinBoxConfig final {
    T default_step;///< value adjust step default value
    T step_adjust; ///< adjust step of step
};

}// namespace kf::internal

namespace kf::ui::widget {

struct SpinBoxTag {};

/// @brief Spin box for adjusting numeric values with different modes
/// @tparam U UI Traits Type
/// @tparam T Numeric type for spin box value (must be arithmetic)
/// @tparam A Adjuster type
template<implements<UiTraitsTag> U, arithmetic T, implements<typename U::AdjusterTag> A> struct SpinBox :

    SpinBoxTag,
    U::Widget,
    mixin::ValueCallbacked<T>,
    mixin::Configured<internal::SpinBoxConfig<T>>

{
    using AdjusterImpl = A;
    using Config = internal::SpinBoxConfig<T>;

    explicit constexpr SpinBox(Config const &config, T default_value = T{}, Style style = Style::defaults()) noexcept :
        U::Widget{style}, mixin::ValueCallbacked<T>{default_value}, mixin::Configured<Config>{config}, _step(config.default_step) {}

    /// @brief Toggle between value adjustment and step adjustment modes
    Request onClick() noexcept override {
        _is_step_setting_mode = not _is_step_setting_mode;

        return Request::Redraw;
    }

    /// @brief Adjust value or step based on current mode
    /// @param event_value Adjustment scale
    Request onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        if (_is_step_setting_mode) {
            _step = U::template GeometricAdjuster<T>::adjust(_step, this->config().step_adjust, event_value);
        } else {
            this->value(AdjusterImpl::adjust(this->value(), _step, event_value));
        }

        return Request::Redraw;
    }

    /// @brief Render current value or step size based on mode
    void doRender(typename U::RendererImpl &render) const noexcept override {
        if (_is_step_setting_mode) {
            render.background(Color::Warning);
        }

        render.beginBlock(Block::Alternative);

        if (_is_step_setting_mode) {
            render.decoration(Decoration::Colon);
            render.value(_step);
        } else {
            render.value(this->value());
        }

        render.endBlock(Block::Alternative);
    }

private:
    T _step;                          ///< Current step size
    bool _is_step_setting_mode{false};///< true when adjusting step size, false when adjusting value
};

}// namespace kf::ui::widget