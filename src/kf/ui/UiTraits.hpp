// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::internal {

template<typename T> struct step_adjuster_min_step;

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
template<typename W> struct UiTraits : UiTraitsTag {
    KF_CHECK_IMPL(W, ::kf::ui::widget::WidgetTag);

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
    template<typename Impl, typename T> struct Adjuster : AdjusterTag {

        /// @brief Applies an adjustment to a value.
        /// @param value    The current value.
        /// @param step     The step size.
        /// @param direction Direction: -1 (decrease), 0 (no change), +1 (increase).
        /// @return The adjusted value.
        [[nodiscard]] static constexpr T adjust(T value, T step, int direction) noexcept {
            return Impl::adjustImpl(value, step, direction);
        }
    };

    /// @brief Arithmetic mode: value += direction * step
    template<typename T> struct ArithmeticAdjuster final : Adjuster<ArithmeticAdjuster<T>, T> {
    private:
        KF_IMPL(Adjuster<ArithmeticAdjuster<T>, T>);
        static constexpr T adjustImpl(T value, T step, int direction) noexcept {
            return value + direction * step;
        }
    };

    /// @brief ArithmeticPositiveOnly mode: value += direction * step, clamp >= 0
    template<typename T> struct ArithmeticPositiveOnlyAdjuster final : Adjuster<ArithmeticPositiveOnlyAdjuster<T>, T> {
    private:
        KF_IMPL(Adjuster<ArithmeticPositiveOnlyAdjuster<T>, T>);
        static constexpr T adjustImpl(T value, T step, int direction) noexcept {
            return max(0, ArithmeticAdjuster<T>::adjust(value, step, direction));
        }
    };

    /// @brief Geometric mode: value *= step for positive direction, /= for negative
    template<typename T> struct GeometricAdjuster final : Adjuster<GeometricAdjuster<T>, T> {
    private:
        KF_IMPL(Adjuster<GeometricAdjuster<T>, T>);
        static constexpr T adjustImpl(T value, T step, int direction) noexcept {
            return (direction == 0) ? value : ((direction > 0) ? (value * step) : max(value / step, internal::step_adjuster_min_step<T>::value));
        }
    };
};

}// namespace kf::ui