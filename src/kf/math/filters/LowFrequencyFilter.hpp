// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
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
    TrivialOption<ValueType> _filtered{none};

    using This = LowFrequencyFilter<ValueType>;

    KF_IMPL(Filter<This, ValueType>);
    ValueType calcImpl(const ValueType &x) noexcept {
        if (_filtered.isNone()) {
            goto set;
        }

        if (this->config().factor == 1.0f) { goto set; }

        _filtered.unwrap() = _filtered.unwrap() * (1.0f - this->config().factor) + x * this->config().factor;
        goto ret;

    set:
        _filtered = someTrivial(x);
    ret:
        return _filtered.unwrap();
    }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _first_step = true;
    }
};

}// namespace kf::math::filters