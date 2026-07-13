// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/mixin/Configurable.hpp"
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
    TrivialOption<ValueType> _filtered{none};

    using This = ExponentialFilter<ValueType>;

    KF_IMPL(Filter<This, ValueType>);
    ValueType calcImpl(const ValueType &value) noexcept {
        if (_filtered.isNone()) {
            _filtered = someTrivial(value);
        } else {
            _filtered.unwrap() += (value - _filtered.unwrap()) * this->config().factor;
        }
        return _filtered.unwrap();
    }

    KF_IMPL_RESETTABLE(This);
    constexpr void resetImpl() noexcept {
        _filtered = none;
    }
};

}// namespace kf::filter