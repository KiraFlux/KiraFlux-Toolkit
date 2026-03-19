// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/math/units.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf::math::filters {

/// @brief Complementary filter for sensor fusion
/// @tparam T Data type (typically float or vector type)
/// @note Combines low-frequency and high-frequency sensor data using weighted average
template<typename T> struct ComplementaryFilter final : kf::mixin::Resettable<ComplementaryFilter<T>> {

    /// @brief Construct complementary filter instance
    /// @param alpha Filter coefficient (higher = more trust in prediction)
    /// @note alpha=0.0: trust only measurement, alpha=1.0: trust only prediction
    explicit ComplementaryFilter(f32 alpha) noexcept :
        _alpha{alpha} {}

    /// @brief Calculate filtered value from measurement and rate of change
    /// @param x Current measurement value
    /// @param dx Rate of change (derivative) of the value
    /// @param dt Time step in seconds since last update
    /// @return Filtered value combining prediction and measurement
    [[nodiscard]] const T &calc(T x, T dx, Seconds dt) noexcept {
        if (_first_step) {
            _first_step = false;
            _filtered = x;
        } else {
            T prediction = _filtered + dx * dt;
            _filtered = _alpha * prediction + (1.0f - _alpha) * x;
        }

        return _filtered;
    }

private:
    const f32 _alpha;      ///< Filter coefficient for prediction (0.0 to 1.0)
    T _filtered{};         ///< Current filtered value
    bool _first_step{true};///< First iteration flag for initialization

    // impl
    using This = ComplementaryFilter<T>;

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _first_step = true;// Reset filter state (next calc will reinitialize with measurement)
    }
};

}// namespace kf::math::filters