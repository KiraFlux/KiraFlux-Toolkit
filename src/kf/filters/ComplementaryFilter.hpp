// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

struct ComplementaryFilterConfig final {
    f32 factor;///< Filter coefficient for prediction (0.0 to 1.0)
};

}// namespace kf::internal

namespace kf::filters {

/// @brief Complementary filter for sensor fusion
/// @tparam T Data type (typically float or vector type)
/// @note Combines low-frequency and high-frequency sensor data using weighted average
template<typename T> struct ComplementaryFilter final :

    mixin::Configurable<internal::ComplementaryFilterConfig>,
    mixin::NonCopyable,
    mixin::Resettable<ComplementaryFilter<T>>

{
    using ValueType = T;
    using Config = internal::ComplementaryFilterConfig;

    using mixin::Configurable<Config>::Configurable;

    /// @brief Calculate filtered value from measurement and rate of change
    /// @param x Current measurement value
    /// @param dx Rate of change (derivative) of the value
    /// @param dt Time step in seconds since last update
    /// @return Filtered value combining prediction and measurement
    [[nodiscard]] const ValueType &calc(ValueType x, ValueType dx, Seconds dt) noexcept {
        if (_filtered.isNone()) {
            _filtered = someTrivial(x);
        } else {
            const auto prediction = _filtered + dx * dt;
            _filtered = someTrivial(this->config().factor * prediction + (1.0f - this->config().factor) * x);
        }

        return _filtered.unwrap();
    }

private:
    TrivialOption<ValueType> _filtered{none};

    using This = ComplementaryFilter<ValueType>;

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _filtered = none;
    }
};

}// namespace kf::filters