// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/attributes.hpp"
#include "kf/math/units.hpp"

namespace kf {

/// @brief Complementary filter for sensor fusion
/// @tparam T Data type (typically float or vector type)
/// @note Combines low-frequency and high-frequency sensor data using weighted average
template<typename T> struct ComplementaryFilter {

private:
    const f32 alpha;          ///< Filter coefficient for prediction (0.0 to 1.0)
    const f32 one_minus_alpha;///< Complementary coefficient (1.0 - alpha)
    T filtered{};             ///< Current filtered value
    bool first_step{true};    ///< First iteration flag for initialization

public:
    /// @brief Construct complementary filter instance
    /// @param alpha Filter coefficient (higher = more trust in prediction)
    /// @note alpha=0.0: trust only measurement, alpha=1.0: trust only prediction
    explicit ComplementaryFilter(f32 alpha) noexcept:
        alpha{alpha}, one_minus_alpha{1.0f - alpha} {}

    /// @brief Calculate filtered value from measurement and rate of change
    /// @param x Current measurement value
    /// @param dx Rate of change (derivative) of the value
    /// @param dt Time step in seconds since last update
    /// @return Filtered value combining prediction and measurement
    kf_nodiscard const T &calc(T x, T dx, Seconds dt) noexcept {
        if (first_step) {
            first_step = false;
            filtered = x;
        } else {
            T prediction = filtered + dx * dt;
            filtered = alpha * prediction + one_minus_alpha * x;
        }

        return filtered;
    }

    /// @brief Reset filter state (next calc will reinitialize with measurement)
    void reset() noexcept {
        first_step = true;
    }
};

}// namespace kf