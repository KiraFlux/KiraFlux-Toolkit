// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"

#include "kf/math/filters/Filter.hpp"

namespace kf::math::filters {

// LowFrequencyFilter
namespace internal::lff {
struct Config final : mixin::NonCopyable {
    f32 factor;///< Smoothing factor (0.0 to 1.0)
};
}// namespace internal::lff

/// @brief Low-frequency filter (first-order low-pass)
/// @tparam T Scalar type (typically float or integer)
/// @note Uses exponential smoothing to attenuate high-frequency noise
template<typename T> struct LowFrequencyFilter : Filter<LowFrequencyFilter<T>, T>, mixin::Configurable<internal::lff::Config> {
    using ValueType = T;
    using Config = internal::lff::Config;

    using mixin::Configurable<Config>::Configurable;

private:
    ValueType _filtered{};  ///< Current filtered value
    bool _first_step{false};///< First sample flag for initialization

    // impl
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