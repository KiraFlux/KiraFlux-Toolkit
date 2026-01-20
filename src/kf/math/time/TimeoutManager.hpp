// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/attributes.hpp"
#include "kf/math/units.hpp"

namespace kf {

/// @brief Service for tracking timeout expiration moments
/// @note Manages timeout intervals and checks for expiration
struct TimeoutManager final {

private:
    Milliseconds timeout;        ///< Timeout duration in milliseconds
    Milliseconds next_timeout{0};///< Timestamp when timeout will expire

public:
    /// @brief Construct timeout manager instance
    /// @param timeout_duration Timeout duration in milliseconds
    explicit TimeoutManager(Milliseconds timeout_duration) :
        timeout{timeout_duration} {}

    /// @brief Update timeout expiration time
    /// @param now Current time in milliseconds
    /// @note Sets next timeout to current time plus configured duration
    void update(Milliseconds now) {
        next_timeout = now + timeout;
    }

    /// @brief Check if timeout has expired
    /// @param now Current time in milliseconds
    /// @return true if timeout has expired, false otherwise
    kf_nodiscard inline bool expired(Milliseconds now) const { return now >= next_timeout; }
};

}// namespace kf
