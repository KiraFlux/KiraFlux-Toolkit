// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

#include "kf/math/filters/Filter.hpp"

namespace kf::math::filters {

/// @brief Low-frequency filter (first-order low-pass)
/// @tparam T Scalar type (typically float or integer)
/// @note Uses exponential smoothing to attenuate high-frequency noise
template<typename T> struct LowFrequencyFilter : Filter<LowFrequencyFilter<T>, T> {
    using ValueType = T;

    struct Config {
        f32 factor;///< Smoothing factor (0.0 to 1.0)
    };

    explicit LowFrequencyFilter(const Config &config) noexcept :
        _config{config} {}

private:
    const Config &_config;
    ValueType _filtered{};  ///< Current filtered value
    bool _first_step{false};///< First sample flag for initialization

    // impl
    using This = LowFrequencyFilter<ValueType>;

    KF_IMPL_FILTER(this, ValueType);
    ValueType calcImpl(const ValueType &x) noexcept {
        if (_first_step) {
            _first_step = false;
            goto set;
        }

        if (_config.factor == 1.0f) { goto set; }

        _filtered = _filtered * (1.0f - _config.factor) + x * _config.factor;
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