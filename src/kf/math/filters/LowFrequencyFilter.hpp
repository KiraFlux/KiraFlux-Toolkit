// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/core/attributes.hpp"


namespace kf {

/// @brief Low-frequency filter (first-order low-pass)
/// @tparam T Scalar type (typically float or integer)
/// @note Uses exponential smoothing to attenuate high-frequency noise
template<typename T> struct LowFrequencyFilter {

private:
    const f32 alpha;          ///< Smoothing factor (0.0 to 1.0)
    const f32 one_minus_alpha;///< Complementary coefficient (1.0 - alpha)
    T filtered{};             ///< Current filtered value
    bool first_step{false};   ///< First sample flag for initialization

public:
    /// @brief Construct low-frequency filter instance
    /// @param alpha Smoothing factor (higher = more smoothing, slower response)
    /// @note alpha=0.0: output never changes, alpha=1.0: no filtering (direct pass-through)
    explicit LowFrequencyFilter(f32 alpha) noexcept:
        alpha{alpha}, one_minus_alpha{1.0f - alpha} {}

    /// @brief Update filter with new sample
    /// @param x New input value
    /// @return Current filtered value
    kf_nodiscard const T &calc(const T &x) noexcept {
        if (first_step) {
            first_step = false;
            filtered = x;
            return filtered;
        }

        if (alpha == 1.0) {
            filtered = x;
            return filtered;
        }

        filtered = filtered * one_minus_alpha + x * alpha;
        return filtered;
    }

    /// @brief Reset filter state (next sample will initialize filter)
    void reset() noexcept{
        first_step = true;
    }
};

}// namespace kf