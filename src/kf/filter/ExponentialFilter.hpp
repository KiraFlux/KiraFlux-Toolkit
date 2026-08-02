// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    filter/ExponentialFilter.hpp
/// @brief   Exponential moving average (EMA) filter.

#pragma once

#include "kf/Option.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

#include "kf/filter/Filter.hpp"

namespace kf::internal {

struct ExponentialFilterConfig final {
    f32 factor;///< Smoothing factor (0.0 to 1.0, higher = faster response)
};

}// namespace kf::internal

namespace kf::filter {

/// @brief Exponential moving average filter (EMA)
/// @tparam T Data type
/// @note Simple first-order IIR filter for smoothing noisy signals
template<arithmetic T> struct ExponentialFilter :

    Filter<ExponentialFilter<T>, T>,
    mixin::Configured<internal::ExponentialFilterConfig>

{
    using ValueType = T;
    using Config = internal::ExponentialFilterConfig;

    using mixin::Configured<Config>::Configured;

private:
    Option<ValueType> _filtered{none};

    KF_IMPL_FILTER(ExponentialFilter<ValueType>, ValueType);

    ValueType calcImpl(ValueType const &value) noexcept {
        if (_filtered.isNone()) {
            _filtered = some(value);
        } else {
            _filtered.unwrap() += (value - _filtered.unwrap()) * this->config().factor;
        }
        return _filtered.unwrap();
    }

    constexpr void resetImpl() noexcept {
        _filtered = none;
    }
};

}// namespace kf::filter