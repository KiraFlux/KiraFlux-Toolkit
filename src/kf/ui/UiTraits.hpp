// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    ui/UiTraits.hpp
/// @brief   UI traits defining Widget, Renderer, Event, and adjusters.

#pragma once

#include "kf/concepts.hpp"
#include "kf/math.hpp"

#include "kf/ui/widget/Widget.hpp"

namespace kf::internal {

template<arithmetic T> struct step_adjuster_min_step;

template<> struct step_adjuster_min_step<int> {
    static constexpr int value{1};
};

template<> struct step_adjuster_min_step<float> {
    static constexpr float value{1e-3f};
};

template<> struct step_adjuster_min_step<double> {
    static constexpr double value{1e-6};
};

}// namespace kf::internal

namespace kf::ui {

struct UiTraitsTag {};

/// @brief UI Traits
/// @tparam W Widget Base implementation (Must inherit from `::kf::ui::widget::WidgetTag` and should be like `::kf::ui::widget::Widget<R, E>`)
template<implements<widget::WidgetTag> W> struct UiTraits : UiTraitsTag {

    /// @brief Widget Base class
    struct Widget : W {
        using W::W;
    };

    /// @brief Render system implementation
    using RendererImpl = typename Widget::RendererImpl;

    /// @brief Event type
    using EventImpl = typename Widget::EventImpl;

    struct AdjusterTag {};

    /// @brief CRTP base for value adjustment strategies
    /// @tparam Impl The derived adjustment class.
    /// @tparam T   The numeric type to adjust.
    template<typename Impl, arithmetic T> struct Adjuster : AdjusterTag {

        /// @brief Applies an adjustment to a value.
        /// @param value    The current value.
        /// @param step     The step size.
        /// @param direction Direction: -1 (decrease), 0 (no change), +1 (increase).
        /// @return The adjusted value.
        [[nodiscard]] static constexpr T adjust(T value, T step, int direction) noexcept {
            return Impl::adjustImpl(value, step, direction);
        }
    };

#define KF_IMPL_ADJUSTER(...) friend struct Adjuster<__VA_ARGS__>

    /// @brief Arithmetic mode: value += direction * step
    template<arithmetic T> struct ArithmeticAdjuster final : Adjuster<ArithmeticAdjuster<T>, T> {
    private:
        KF_IMPL_ADJUSTER(ArithmeticAdjuster<T>, T);
        static constexpr T adjustImpl(T value, T step, int direction) noexcept {
            return value + direction * step;
        }
    };

    /// @brief ArithmeticPositiveOnly mode: value += direction * step, clamp >= 0
    template<arithmetic T> struct ArithmeticPositiveOnlyAdjuster final : Adjuster<ArithmeticPositiveOnlyAdjuster<T>, T> {
    private:
        KF_IMPL_ADJUSTER(ArithmeticPositiveOnlyAdjuster<T>, T);
        static constexpr T adjustImpl(T value, T step, int direction) noexcept {
            return max(0, ArithmeticAdjuster<T>::adjust(value, step, direction));
        }
    };

    /// @brief Geometric mode: value *= step for positive direction, /= for negative
    template<arithmetic T> struct GeometricAdjuster final : Adjuster<GeometricAdjuster<T>, T> {
    private:
        KF_IMPL_ADJUSTER(GeometricAdjuster<T>, T);
        static constexpr T adjustImpl(T value, T step, int direction) noexcept {
            return (direction == 0) ? value : ((direction > 0) ? (value * step) : math::max(value / step, internal::step_adjuster_min_step<T>::value));
        }
    };
};

}// namespace kf::ui