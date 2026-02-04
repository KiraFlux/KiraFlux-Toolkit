// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf {// NOLINT(*-concat-nested-namespaces) // for c++11 capability
namespace ui {// NOLINT(*-concat-nested-namespaces)
namespace detail {// NOLINT(*-concat-nested-namespaces)


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


template<typename T> struct step_adjuster_default_step;

template<> struct step_adjuster_default_step<int> {
    static constexpr int value{1};
};

template<> struct step_adjuster_default_step<float> {
    static constexpr float value{0.1f};
};

template<> struct step_adjuster_default_step<double> {
    static constexpr double value{0.01};
};


/// @brief Step adjustment with type-specific protection
template<typename T> struct StepAdjuster {
    static constexpr T step_multiplier{static_cast<T>(10)};
    static constexpr T min_step{step_adjuster_min_step<T>::value};
    static constexpr T default_step{step_adjuster_default_step<T>::value};

    static void adjust(T &step, int direction) noexcept {
        if (direction > 0) {
            step *= step_multiplier;
        } else {
            step /= step_multiplier;
            // Protection for integral types
            if (step < min_step) { step = min_step; }
        }
    }
};

}
}
}