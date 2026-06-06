// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

#include "kf/math/filters/Filter.hpp"

namespace kf::math::filters {

namespace internal {

struct ExponentialFilterConfig final {
    f32 factor;///< Smoothing factor (0.0 to 1.0, higher = faster response)
};

}// namespace internal

/// @brief Exponential moving average filter (EMA)
/// @tparam T Data type (typically float or integer)
/// @note Simple first-order IIR filter for smoothing noisy signals
template<typename T> struct ExponentialFilter :

    Filter<ExponentialFilter<T>, T>,
    mixin::Configurable<internal::ExponentialFilterConfig>

{
    using ValueType = T;
    using Config = internal::ExponentialFilterConfig;

    using mixin::Configurable<Config>::Configurable;

private:
    ValueType _current_filtered{};
    bool _first_step{true};

    using This = ExponentialFilter<ValueType>;

    KF_IMPL(Filter<This, ValueType>);
    ValueType calcImpl(const ValueType &value) noexcept {
        if (_first_step) {
            _current_filtered = value;
        } else {
            _current_filtered += (value - _current_filtered) * this->config().factor;
        }
        return _current_filtered;
    }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _first_step = true;
    }
};

}// namespace kf::math::filters