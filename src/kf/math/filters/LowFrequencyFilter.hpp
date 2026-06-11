// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

#include "kf/math/filters/Filter.hpp"

namespace kf::internal {

struct LowFrequencyFilterConfig final {
    f32 factor;///< Smoothing factor (0.0 to 1.0)
};

}// namespace kf::internal

namespace kf::math::filters {

/// @brief First-order low-pass filter (exponential smoothing).
/// @tparam T Scalar type (e.g., float, double).
/// @note Attenuates high-frequency noise. The `factor` determines smoothing:
///       factor = 1.0 -> no filtering (output = input)
///       factor = 0.0 -> output stays constant (infinite smoothing)
template<typename T> struct LowFrequencyFilter :

    Filter<LowFrequencyFilter<T>, T>,
    mixin::Configurable<internal::LowFrequencyFilterConfig>

{
    using ValueType = T;
    using Config = internal::LowFrequencyFilterConfig;

    using mixin::Configurable<Config>::Configurable;

private:
    ValueType _filtered{};  ///< Current filtered value
    bool _first_step{false};///< First sample flag for initialization

    using This = LowFrequencyFilter<ValueType>;

    KF_IMPL(Filter<This, ValueType>);
    ValueType calcImpl(const ValueType &x) noexcept {
        if (_first_step) {
            _first_step = false;
            goto set;
        }

        if (this->config().factor == 1.0f) { goto set; }

        _filtered = _filtered * (1.0f - this->config().factor) + x * this->config().factor;
        goto ret;

    set:
        _filtered = x;
    ret:
        return _filtered;
    }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _first_step = true;
    }
};

}// namespace kf::math::filters