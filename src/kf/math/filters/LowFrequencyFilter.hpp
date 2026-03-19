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

    struct Config {
        f32 factor;///< Smoothing factor (0.0 to 1.0)
    };

    explicit LowFrequencyFilter(const Config &config) noexcept :
        _config{config} {}

private:
    const Config &_config;
    T _filtered{};          ///< Current filtered value
    bool _first_step{false};///< First sample flag for initialization

    // impl

    friend struct kf::math::filters::Filter<LowFrequencyFilter<T>, T>;

    T calcImpl(const T &x) noexcept {
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

    friend struct kf::mixin::Resettable<ExponentialFilter<T>>;

    void resetImpl() noexcept {
        _first_step = true;
    }
};

}// namespace kf::math::filters