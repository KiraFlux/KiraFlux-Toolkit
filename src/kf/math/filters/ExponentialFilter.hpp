// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

#include "kf/math/filters/Filter.hpp"

namespace kf::math::filters {

/// @brief Exponential moving average filter (EMA)
/// @tparam T Data type (typically float or integer)
/// @note Simple first-order IIR filter for smoothing noisy signals
template<typename T> struct ExponentialFilter : Filter<ExponentialFilter<T>, T> {
    using ValueType = T;

    struct Config {
        f32 factor;///< Smoothing factor (0.0 to 1.0, higher = faster response)
    };

    constexpr explicit ExponentialFilter(const Config &config) noexcept : _config{config} {}

private:
    const Config &_config;
    ValueType _current_filtered{};

    // impl
    using This = ExponentialFilter<ValueType>;

    KF_IMPL_FILTER(This, ValueType);
    ValueType calcImpl(const ValueType &value) noexcept {
        _current_filtered += (value - _current_filtered) * _config.factor;
        return _current_filtered;
    }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _current_filtered = ValueType{};
    }
};

}// namespace kf::math::filters