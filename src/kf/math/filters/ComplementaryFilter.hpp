// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/math/units.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf::math::filters {

// ComplementaryFilter
namespace internal::cf {
struct Config {
    f32 factor;///< Filter coefficient for prediction (0.0 to 1.0)
};

}// namespace internal::cf

/// @brief Complementary filter for sensor fusion
/// @tparam T Data type (typically float or vector type)
/// @note Combines low-frequency and high-frequency sensor data using weighted average
template<typename T>
struct ComplementaryFilter final : mixin::Configurable<internal::cf::Config>,
                                   mixin::NonCopyable,
                                   mixin::Resettable<ComplementaryFilter<T>> {
    using ValueType = T;
    using Config = internal::cf::Config;

    using mixin::Configurable<Config>::Configurable;

    /// @brief Calculate filtered value from measurement and rate of change
    /// @param x Current measurement value
    /// @param dx Rate of change (derivative) of the value
    /// @param dt Time step in seconds since last update
    /// @return Filtered value combining prediction and measurement
    [[nodiscard]] const ValueType &calc(ValueType x, ValueType dx, Seconds dt) noexcept {
        if (_first_step) {
            _first_step = false;
            _filtered = x;
        } else {
            const auto prediction = _filtered + dx * dt;
            _filtered = this->config().factor * prediction + (1.0f - this->config().factor) * x;
        }

        return _filtered;
    }

private:
    ValueType _filtered{}; ///< Current filtered value
    bool _first_step{true};///< First iteration flag for initialization

    // impl
    using This = ComplementaryFilter<ValueType>;

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _first_step = true;// Reset filter state (next calc will reinitialize with measurement)
    }
};

}// namespace kf::math::filters