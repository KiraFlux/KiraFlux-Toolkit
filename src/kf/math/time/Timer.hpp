// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/attributes.hpp"
#include "kf/math/units.hpp"

namespace kf {

/// @brief Periodic timer for scheduling recurring events
/// @note Checks if specified time interval has elapsed since last trigger
struct Timer {

    Milliseconds period;///< Timer interval in milliseconds

private:
    Milliseconds last{0};///< Timestamp of last trigger

public:
    /// @brief Construct timer with period in milliseconds
    /// @param period Time interval in milliseconds
    explicit Timer(Milliseconds period) :
        period{period} {}

    /// @brief Construct timer with frequency in Hz
    /// @param frequency Trigger frequency in Hertz
    explicit Timer(Hertz frequency) :
        period{static_cast<Milliseconds>(1000 / frequency)} {}

    /// @brief Check if timer interval has elapsed
    /// @param now Current time in milliseconds
    /// @return true if period has elapsed, false otherwise
    /// @note Resets internal timestamp when interval elapses
    kf_nodiscard bool ready(Milliseconds now) {
        const auto delta = now - last;

        if (delta < period) {
            return false;
        }

        last = now;

        return true;
    }
};

}// namespace kf