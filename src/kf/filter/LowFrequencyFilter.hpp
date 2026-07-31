// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

#include "kf/filter/Filter.hpp"

namespace kf::internal {

struct LowFrequencyFilterConfig final {
    f32 factor;///< Smoothing factor (0.0 to 1.0)
};

}// namespace kf::internal

namespace kf::filter {

/// @brief First-order low-pass filter (exponential smoothing).
/// @note Attenuates high-frequency noise. The `factor` determines smoothing:
///       factor = 1.0 -> no filtering (output = input)
///       factor = 0.0 -> output stays constant (infinite smoothing)
template<arithmetic T> struct LowFrequencyFilter :

    Filter<LowFrequencyFilter<T>, T>,
    mixin::Configured<internal::LowFrequencyFilterConfig>

{
    using ValueType = T;
    using Config = internal::LowFrequencyFilterConfig;

    using mixin::Configured<Config>::Configured;

private:
    Option<ValueType> _filtered{none};

    KF_IMPL_FILTER(LowFrequencyFilter<ValueType>, ValueType);

    ValueType calcImpl(ValueType const &x) noexcept {
        if (_filtered.isNone()) {
            goto set;
        }

        if (this->config().factor == 1.0f) { goto set; }

        _filtered.unwrap() = _filtered.unwrap() * (1.0f - this->config().factor) + x * this->config().factor;
        goto ret;

    set:
        _filtered = x;
    ret:
        return _filtered.unwrap();
    }

    constexpr void resetImpl() noexcept {
        _first_step = true;
    }
};

}// namespace kf::filter