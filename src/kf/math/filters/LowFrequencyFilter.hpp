// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

namespace kf::math::filters {

/// @brief Low-frequency filter (first-order low-pass)
/// @tparam T Scalar type (typically float or integer)
/// @note Uses exponential smoothing to attenuate high-frequency noise
template<typename T> struct LowFrequencyFilter {

private:
    const f32 _alpha;          ///< Smoothing factor (0.0 to 1.0)
    const f32 _one_minus_alpha;///< Complementary coefficient (1.0 - alpha)
    T _filtered{};             ///< Current filtered value
    bool _first_step{false};   ///< First sample flag for initialization

public:
    /// @brief Construct low-frequency filter instance
    /// @param alpha Smoothing factor (higher = more smoothing, slower response)
    /// @note alpha=0.0: output never changes, alpha=1.0: no filtering (direct pass-through)
    explicit LowFrequencyFilter(f32 alpha) noexcept :
        _alpha{alpha}, _one_minus_alpha{1.0f - alpha} {}

    /// @brief Update filter with new sample
    /// @param x New input value
    /// @return Current filtered value
    [[nodiscard]] const T &calc(const T &x) noexcept {
        if (_first_step) {
            _first_step = false;
            goto set;
        }

        if (_alpha == 1.0) { goto set; }

        _filtered = _filtered * _one_minus_alpha + x * _alpha;
        goto ret;

    set:
        _filtered = x;
    ret:
        return _filtered;
    }

    /// @brief Reset filter state (next sample will initialize filter)
    void reset() noexcept {
        _first_step = true;
    }
};

}// namespace kf