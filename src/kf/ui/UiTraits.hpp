// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/ui/Event.hpp"
#include "kf/ui/render/Render.hpp"

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

template<typename R, typename E> struct UiTraits : UiTraitsTag {
    KF_CHECK_IMPL(R, ::kf::ui::render::RenderTag);
    using RenderImpl = R;

    KF_CHECK_IMPL(E, ::kf::ui::EventTag);
    using EventImpl = E;

    struct AdjusterTag {};

    /// @brief CRTP base for value adjustment strategies used by SpinBox.
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