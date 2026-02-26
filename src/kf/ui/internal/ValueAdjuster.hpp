// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/internal/StepMode.hpp"

namespace kf::ui::internal {

/// @brief Generic spin box value adjustment
template<typename T, StepMode M> struct ValueAdjuster;

/// @brief Arithmetic mode: value += direction * step
template<typename T> struct ValueAdjuster<T, StepMode::Arithmetic> {
    static void adjust(T &value, T step, int direction) noexcept {
        value += direction * step;
    }
};

/// @brief ArithmeticPositiveOnly mode: value += direction * step, clamp >= 0
template<typename T> struct ValueAdjuster<T, StepMode::ArithmeticPositiveOnly> {
    static void adjust(T &value, T step, int direction) noexcept {
        value += direction * step;
        if (value < 0) { value = 0; }
    }
};

/// @brief Geometric mode: value *= step for positive direction, /= for negative
template<typename T> struct ValueAdjuster<T, StepMode::Geometric> {
    static void adjust(T &value, T step, int direction) noexcept {
        if (direction > 0) {
            value *= step;
        } else {
            value /= step;
        }
    }
};

}// namespace kf::ui::internal