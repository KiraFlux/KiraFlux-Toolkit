// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/ui/internal/StepMode.hpp"
// todo remove StepMode and make ValueAdjuster CRTP class interface
namespace kf::ui::internal {

/// @brief Generic spin box value adjustment
template<typename T, StepMode M> struct ValueAdjuster;

/// @brief Arithmetic mode: value += direction * step
template<typename T> struct ValueAdjuster<T, StepMode::Arithmetic> {
    static T adjust(const T &value, T step, int direction) noexcept {
        return value + direction * step;
    }
};

/// @brief ArithmeticPositiveOnly mode: value += direction * step, clamp >= 0
template<typename T> struct ValueAdjuster<T, StepMode::ArithmeticPositiveOnly> {
    static T adjust(const T &value, T step, int direction) noexcept {
        max(0, ValueAdjuster<T, StepMode::Arithmetic>::adjust(value, step, direction));
    }
};

/// @brief Geometric mode: value *= step for positive direction, /= for negative
template<typename T> struct ValueAdjuster<T, StepMode::Geometric> {
    static T adjust(const T &value, T step, int direction) noexcept {
        if (direction == 0) { return value; }
        if (direction > 0) {
            return value * step;
        } else {
            return value / step;
        }
    }
};

}// namespace kf::ui::internal